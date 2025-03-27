package com.example.composedemo

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.BorderStroke
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.interaction.MutableInteractionSource
import androidx.compose.foundation.interaction.collectIsPressedAsState
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.text.BasicTextField
import androidx.compose.material.Button
import androidx.compose.material.BottomNavigation
import androidx.compose.material.BottomNavigationItem
import androidx.compose.material.FloatingActionButton
import androidx.compose.material.Icon
import androidx.compose.material.IconButton
import androidx.compose.material.MaterialTheme
import androidx.compose.material.Scaffold
import androidx.compose.material.Surface
import androidx.compose.material.Text
import androidx.compose.material.TopAppBar
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ArrowBack
import androidx.compose.material.icons.filled.Close
import androidx.compose.material.icons.filled.Favorite
import androidx.compose.material.icons.filled.KeyboardArrowUp
import androidx.compose.material.icons.filled.Search
import androidx.compose.runtime.Composable
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.core.view.WindowCompat
import com.google.accompanist.systemuicontroller.rememberSystemUiController

data class Item(val name: String, val icon: ImageVector)

class MainActivity : ComponentActivity() {
  override fun onCreate(savedInstanceState: Bundle?) {
    super.onCreate(savedInstanceState)

    WindowCompat.setDecorFitsSystemWindows(window, false)

    setContent {
      android.util.Log.i("Demo-setContent", "Demo-setContent")
      val systemUiController = rememberSystemUiController()
      val useDarkIcons = MaterialTheme.colors.isLight
      DisposableEffect(systemUiController) {
        systemUiController.setSystemBarsColor(color = Color.Transparent, darkIcons = useDarkIcons)
        onDispose {}
      }

      val names = listOf("Android", "Kotlin", "Compose", "Jetpack", "Google")
      var name by mutableStateOf(names.random())
      var count by mutableStateOf((1..5).random())
      var color by mutableStateOf(
        Color(
          red = (0..255).random(), green = (0..255).random(), blue = (0..255).random()
        )
      )
      var greeting by mutableStateOf(true)

      // LaunchedEffect(Unit) {
      //   while (true) {
      //     kotlinx.coroutines.delay(1000) // Ensure the delay function is fully qualified
      //     greeting = !greeting
      //     color =
      //             Color(
      //                     red = (0..255).random(),
      //                     green = (0..255).random(),
      //                     blue = (0..255).random()
      //             )
      //   }
      // }
      var selectedItem by mutableStateOf(0)
      val items = listOf(
        Item("主页", Icons.Filled.Favorite), Item("列表", Icons.Filled.ArrowBack), Item("设置", Icons.Filled.Search)
      )

      ComposeTheme {
        Scaffold(topBar = {
          TopAppBar(title = { Text(text = "Compose Demo") }, navigationIcon = {
            IconButton(onClick = { /*TODO*/ }) {
              Icon(Icons.Filled.ArrowBack, contentDescription = "Back")
            }
          })
        }, floatingActionButton = {
          FloatingActionButton(onClick = { /*TODO*/ }) {
            Icon(Icons.Filled.KeyboardArrowUp, contentDescription = "Up")
          }
        }, bottomBar = {
          BottomNavigation {
            items.forEachIndexed { index, item ->
              BottomNavigationItem(
                selected = selectedItem == index,
                onClick = { selectedItem = index },
                icon = { Icon(imageVector = item.icon, null) },
                alwaysShowLabel = false,
                label = { Text(item.name) })
            }
          }
        }) {
          Greeting(name = name, count = count)
        }

        // A surface container using the 'background' color from the theme
//        Surface(modifier = Modifier.fillMaxSize(), color = color) {
//          if (greeting) Greeting(name = name, count = count)
//          else Text("Compose Demo", modifier = Modifier.systemBarsPadding())
//        }
      }
    }
  }
}

@Composable
fun Greeting(name: String, count: Int) {
  Column(
    modifier = Modifier.fillMaxSize(), // 添加系统栏(状态栏和导航栏)的内边距,
    horizontalAlignment = Alignment.CenterHorizontally, verticalArrangement = Arrangement.Center
  ) {
    repeat(count) {
      Box(modifier = Modifier.weight(1f), contentAlignment = Alignment.TopCenter) {
        Text(text = "Hello $name!")
      }
    }

    // ListWithBug(List(count) { "${(1..100).random()}" })
    Box(
      modifier = Modifier
        .weight(1f)
        .background(Color(0xFFD3D3D3))
        .fillMaxWidth(), contentAlignment = Alignment.Center
    ) {
//      var openDialog by remember { mutableStateOf(false) }
//      if (openDialog) {
//        Dialog(onDismissRequest = {openDialog = false}) {
//          Box(Modifier.size(200.dp, 50.dp).background(Color.Red)) {
//            Text("Dialog")
//          }
//        }
//      }
//      InteractionButton { openDialog = true }
      // TextFieldSample()
      ArticleCard()
    }
  }
}

@Composable
fun ArticleCard() {
  Surface(
    shape = RoundedCornerShape(8.dp), modifier = Modifier
      .padding(horizontal = 12.dp)
      .fillMaxWidth(), elevation = 10.dp
  ) {
    Column(
      modifier = Modifier.padding(12.dp)// 里面内容的外边距
    ) {
      Text(
        text = "Jetpack Compose 是什么？", style = MaterialTheme.typography.h6
      )
      Spacer(Modifier.padding(vertical = 5.dp))
      Text(
        text = "Jetpack Compose 是用于构建原生 Android界面的新工具包。它可简化并加快 Android上的界 面开发, 使用更少的代码、强大的工具和直观的 KotLin API，让应用生动而精彩。"
      )
      Row(
        modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceBetween
      ) {
        IconButton(onClick = {/* TODO */ }) {
          Icon(Icons.Filled.Favorite, null)
        }
        IconButton(onClick = { /* TODO */ }) {
          Icon(Icons.Filled.Favorite, null)
        }
        IconButton(onClick = { /* TODO */ }) {
          Icon(Icons.Filled.Favorite, null)
        }
      }
    }
  }
}

@Composable
fun InteractionButton(onClick: () -> Unit = {}) {
  var interactionSource = remember { MutableInteractionSource() }
  Button(
    onClick = onClick, interactionSource = interactionSource, border = BorderStroke(
      2.dp, if (interactionSource.collectIsPressedAsState().value) Color.Green
      else Color.White
    )
  ) { Text("Click me") }
//  val checkedState = remember { mutableStateOf (true) }
//  Checkbox(
//      checked = checkedState.value,
//      onCheckedChange = { checkedState.value = it },
//      colors = CheckboxDefaults.colors(checkedColor = Color(0xFF0079D3)))
}

@Composable
fun ComposeTheme(content: @Composable () -> Unit) {
  MaterialTheme(
    colors = MaterialTheme.colors.copy(
      primary = Color(0xFF6200EE), primaryVariant = Color(0xFF3700B3), secondary = Color(0xFF03DAC5)
    )
  ) { content() }
}

@Preview(showBackground = true)
@Composable
fun DefaultPreview() {
  ComposeTheme { Greeting("Android", 2) }
}

@Composable
fun TextFieldSample() {
  android.util.Log.i("TextFieldSample", "TextFieldSample")
  var text by remember { mutableStateOf("") }
  // var text by mutableStateOf("")

  // OutlinedTextField(value = text, onValueChange = { text = it }, label = { Text("用户名") })
  BasicTextField(
    value = text, onValueChange = { text = it }, decorationBox = { innerTextField ->
      Row(
        verticalAlignment = Alignment.CenterVertically, modifier = Modifier.padding(vertical = 2.dp, horizontal = 8.dp)
      ) {
        Icon(imageVector = Icons.Filled.Search, contentDescription = "Search")
        Box(contentAlignment = Alignment.CenterStart) {
          if (text.isEmpty()) {
            Text("请输入搜索内容", color = Color.Gray.copy(alpha = 0.5f))
          }
          innerTextField()
        }
        if (text.isNotEmpty()) {
          Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.End) {
            Icon(
              imageVector = Icons.Filled.Close,
              contentDescription = "Clear",
              modifier = Modifier.clickable { text = "" })
          }
        }
      }
    }, modifier = Modifier
      .padding(horizontal = 10.dp)
      .background(Color.White, CircleShape)
      .height(30.dp)
      .fillMaxWidth()
  )
}
