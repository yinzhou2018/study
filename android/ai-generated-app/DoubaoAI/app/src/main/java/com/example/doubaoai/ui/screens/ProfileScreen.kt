package com.example.doubaoai.ui.screens

import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import com.google.accompanist.systemuicontroller.rememberSystemUiController

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ProfileScreen() {
    val systemUiController = rememberSystemUiController()
    val containerColor = MaterialTheme.colorScheme.primaryContainer
    
    SideEffect {
        systemUiController.setStatusBarColor(containerColor)
    }

    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text("个人资料") },
                colors = TopAppBarDefaults.topAppBarColors(
                    containerColor = containerColor,
                    titleContentColor = MaterialTheme.colorScheme.onPrimaryContainer
                )
            )
        }
    ) { paddingValues ->
        LazyColumn(
            modifier = Modifier
                .fillMaxSize()
                .padding(paddingValues),
            horizontalAlignment = Alignment.CenterHorizontally
        ) {
            // 用户头像和基本信息
            item {
                Surface(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(16.dp),
                    color = MaterialTheme.colorScheme.surfaceVariant,
                    shape = MaterialTheme.shapes.medium
                ) {
                    Column(
                        modifier = Modifier.padding(16.dp),
                        horizontalAlignment = Alignment.CenterHorizontally
                    ) {
                        Icon(
                            imageVector = Icons.Default.AccountCircle,
                            contentDescription = "用户头像",
                            modifier = Modifier.size(80.dp),
                            tint = MaterialTheme.colorScheme.primary
                        )
                        Spacer(modifier = Modifier.height(8.dp))
                        Text(
                            text = "用户名",
                            style = MaterialTheme.typography.titleLarge
                        )
                        Text(
                            text = "user@example.com",
                            style = MaterialTheme.typography.bodyMedium,
                            color = MaterialTheme.colorScheme.onSurfaceVariant
                        )
                    }
                }
            }

            // 设置选项列表
            items(getSettingItems()) { item ->
                SettingItem(item)
            }
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SettingItem(item: SettingItem) {
    Card(
        modifier = Modifier
            .fillMaxWidth()
            .padding(horizontal = 16.dp, vertical = 4.dp),
        colors = CardDefaults.cardColors(
            containerColor = MaterialTheme.colorScheme.surface
        )
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Icon(
                imageVector = item.icon,
                contentDescription = item.title,
                tint = MaterialTheme.colorScheme.primary
            )
            Spacer(modifier = Modifier.width(16.dp))
            Column(
                modifier = Modifier.weight(1f)
            ) {
                Text(
                    text = item.title,
                    style = MaterialTheme.typography.titleMedium
                )
                if (item.description.isNotEmpty()) {
                    Text(
                        text = item.description,
                        style = MaterialTheme.typography.bodyMedium,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
            }
            Icon(
                imageVector = Icons.Default.ChevronRight,
                contentDescription = "打开",
                tint = MaterialTheme.colorScheme.onSurfaceVariant
            )
        }
    }
}

data class SettingItem(
    val title: String,
    val description: String = "",
    val icon: androidx.compose.ui.graphics.vector.ImageVector
)

fun getSettingItems(): List<SettingItem> {
    return listOf(
        SettingItem(
            title = "账号安全",
            description = "密码修改、账号绑定",
            icon = Icons.Default.Security
        ),
        SettingItem(
            title = "通知设置",
            description = "推送通知、消息提醒",
            icon = Icons.Default.Notifications
        ),
        SettingItem(
            title = "主题设置",
            description = "深色模式、主题颜色",
            icon = Icons.Default.Palette
        ),
        SettingItem(
            title = "关于我们",
            description = "版本信息、隐私政策",
            icon = Icons.Default.Info
        )
    )
}