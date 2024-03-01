#include <jni.h>
#include <string>
#include <vector>
#include <random>
#include <arm_sve.h>
#include <chrono>

std::vector<float> generateRandomVector(size_t n, float low=0.0, float high=1.0) {
    std::vector<float> result(n);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> distr(low, high);

    for (size_t i = 0; i < n; ++i) {
        result[i] = distr(gen);
    }

    return result;
}

template<typename Func, typename... Args>
long long measureExecutionTime(Func func, int trialCount, Args... args) {
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < trialCount; ++i) {
        func(args...); // Invoke the function with the provided arguments
    }

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

    return duration.count(); // Return the execution time in milliseconds
}

void additionOfProduct(const std::vector<float>& a,
                                     const std::vector<float>& b,
                                     const std::vector<float>& c,
                                     std::vector<float>& result) {
    // Ensure all vectors have the same size
    if (a.size() != b.size() || a.size() != c.size() || a.size() != result.size()) {
        throw std::invalid_argument("All vectors must be of the same size.");
    }

    for (size_t i = 0; i < a.size(); ++i) {
        result[i] = a[i] * b[i] + c[i];
    }
}

void additionOfProductSVE2(const float* a,
                           const float* b,
                           const float* c,
                           float* result,
                           size_t n) {

    // Get the SVE2 vector register length for floats
    size_t vec_len = svcntw();

    for (size_t i = 0; i < n; i += vec_len) {
        // Create a predicate for the current vector segment
        svbool_t pg = svwhilelt_b32(i, n);

        // Load a segment of 'a'
        svfloat32_t va = svld1(pg, &a[i]);

        // Load a segment of 'b'
        svfloat32_t vb = svld1(pg, &b[i]);

        // Load a segment of 'c'
        svfloat32_t vc = svld1(pg, &c[i]);

        // Perform the calculation: a * b + c
        svfloat32_t vresult = svmla_f32_z(pg, vc, va, vb);

        // Store the result
        svst1(pg, &result[i], vresult);
    }
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_armsve2_MainActivity_runCalculations(JNIEnv *env, jobject thiz, jboolean use_sve2,
                                                      jint vector_length) {
    const int trialCount = 1000;

    std::vector<float> vector1 = generateRandomVector(vector_length);
    std::vector<float> vector2 = generateRandomVector(vector_length);
    std::vector<float> offset = generateRandomVector(vector_length);

    std::vector<float> result(vector_length);

    long long executionTime;

    if(use_sve2){
        executionTime = measureExecutionTime(additionOfProductSVE2, trialCount,
                                             vector1.data(), vector2.data(), offset.data(),
                                             result.data(), vector_length);
    }
    else {
        executionTime = measureExecutionTime(additionOfProduct,
                                             trialCount, vector1, vector2, offset, result);
    }

    std::string resultString = "Execution time [ms]: " + std::to_string(executionTime);

    return env->NewStringUTF(resultString.c_str());
}