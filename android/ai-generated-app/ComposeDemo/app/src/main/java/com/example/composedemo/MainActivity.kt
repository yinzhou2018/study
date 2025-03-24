package com.example.composedemo

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material.MaterialTheme
import androidx.compose.material.Surface
import androidx.compose.material.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.tooling.preview.Preview
import androidx.core.view.WindowCompat
import com.google.accompanist.systemuicontroller.rememberSystemUiController
import androidx.compose.ui.graphics.Color
import androidx.compose.foundation.layout.systemBarsPadding
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.remember
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.getValue
import androidx.compose.runtime.setValue
import androidx.compose.runtime.LaunchedEffect
import kotlinx.coroutines.delay

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        
        WindowCompat.setDecorFitsSystemWindows(window, false)
        
        setContent {
          val systemUiController = rememberSystemUiController()
          val useDarkIcons = MaterialTheme.colors.isLight
          DisposableEffect(systemUiController) {
              systemUiController.setSystemBarsColor(
                  color = Color.Transparent,
                  darkIcons = useDarkIcons
              )
              onDispose {}
          }

          val names = listOf("Android", "Kotlin", "Compose", "Jetpack", "Google")
          var name by remember { mutableStateOf(names.random()) }
          var count by remember { mutableStateOf((1..5).random()) }
          var color by remember { mutableStateOf(Color(0xFF000000)) }
          
          // 每秒更新name和count的值
          LaunchedEffect(Unit) {
              while(true) {
                  delay(1000)
                  name = names.random()
                  count = (1..5).random()
                  color = Color(
                      red = (0..255).random(),
                      green = (0..255).random(),
                      blue = (0..255).random() 
                  )
              }
          }

            ComposeTheme {
                // A surface container using the 'background' color from the theme
                Surface(
                    modifier = Modifier.fillMaxSize(),
                    color = color
                ) {
                    Greeting(name = name, count = count)
                }
            }
        }
    }
}

@Composable
fun Greeting(name: String, count: Int) {
    Column(
        modifier = Modifier.fillMaxSize().systemBarsPadding(), // 添加系统栏(状态栏和导航栏)的内边距,
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.Center
    ) {
      repeat(count) {
          Box(
                modifier = Modifier.weight(1f),
                contentAlignment = Alignment.TopCenter
            ) {
                Text(text = "Hello $name!")
            }
        }
    }
}

@Composable
fun ComposeTheme(content: @Composable () -> Unit) {
    MaterialTheme(
        colors = MaterialTheme.colors.copy(
            primary = androidx.compose.ui.graphics.Color(0xFF6200EE),
            primaryVariant = androidx.compose.ui.graphics.Color(0xFF3700B3),
            secondary = androidx.compose.ui.graphics.Color(0xFF03DAC5)
        )
    ) {
        content()
    }
}

@Preview(showBackground = true)
@Composable
fun DefaultPreview() {
    ComposeTheme {
        Greeting("Android", 4)
    }
}