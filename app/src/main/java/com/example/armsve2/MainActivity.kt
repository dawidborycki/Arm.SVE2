package com.example.armsve2

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.widget.Button
import android.widget.CheckBox
import android.widget.EditText
import android.widget.TextView

class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val numericInput = findViewById<EditText>(R.id.numericInput)
        val actionButton = findViewById<Button>(R.id.actionButton)
        val resultLabel = findViewById<TextView>(R.id.resultLabel)
        val useSVE2Checkbox = findViewById<CheckBox>(R.id.controlCheckbox)

        actionButton.setOnClickListener {

            val inputStr = numericInput.text.toString()

            if (inputStr.isNotEmpty()) {
                resultLabel.text = runCalculations(useSVE2Checkbox.isChecked, inputStr.toInt())
            }
            else {
                resultLabel.text = "Invalid input"
            }
        }
    }

    /**
     * A native method that is implemented by the 'armsve2' native library,
     * which is packaged with this application.
     */
    external fun runCalculations(useSVE2: Boolean, vectorLength: Int): String

    companion object {
        // Used to load the 'armsve2' library on application startup.
        init {
            System.loadLibrary("armsve2")
        }
    }
}