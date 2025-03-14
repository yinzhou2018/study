package com.example.doubaoai.ui.screens

import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.hilt.navigation.compose.hiltViewModel
import com.example.doubaoai.data.model.Movie
import com.example.doubaoai.ui.viewmodels.HomeViewModel
import androidx.compose.material3.pullrefresh.PullRefreshIndicator
import androidx.compose.material3.pullrefresh.pullRefresh
import androidx.compose.material3.pullrefresh.rememberPullRefreshState
import com.google.accompanist.systemuicontroller.rememberSystemUiController

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun HomeScreen(
    viewModel: HomeViewModel = hiltViewModel()
) {
    val inTheatersMovies by viewModel.inTheatersMovies.collectAsState()
    val top250Movies by viewModel.top250Movies.collectAsState()
    val isLoading by viewModel.isLoading.collectAsState()
    val error by viewModel.error.collectAsState()
    
    val systemUiController = rememberSystemUiController()
    val containerColor = MaterialTheme.colorScheme.primaryContainer
    
    SideEffect {
        systemUiController.setStatusBarColor(containerColor)
    }
    
    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text("豆包AI") },
                colors = TopAppBarDefaults.topAppBarColors(
                    containerColor = containerColor,
                    titleContentColor = MaterialTheme.colorScheme.onPrimaryContainer
                )
            )
        }
    ) { paddingValues ->
        if (error != null) {
            Box(
                modifier = Modifier.fillMaxSize(),
                contentAlignment = androidx.compose.ui.Alignment.Center
            ) {
                Text(text = error ?: "")
            }
        } else {
            val pullRefreshState = rememberPullRefreshState(
                refreshing = isLoading,
                onRefresh = { viewModel.refresh() }
            )
            
            Box(modifier = Modifier
                .fillMaxSize()
                .pullRefresh(pullRefreshState)
            ) {
                LazyColumn(
                    modifier = Modifier
                        .fillMaxSize()
                        .padding(paddingValues),
                    contentPadding = PaddingValues(16.dp),
                    verticalArrangement = Arrangement.spacedBy(16.dp)
                ) {
                    item {
                        Text(
                            text = "正在热映",
                            style = MaterialTheme.typography.titleLarge,
                            modifier = Modifier.padding(bottom = 8.dp)
                        )
                    }
                    items(inTheatersMovies) { movie ->
                        MovieCard(movie)
                    }
                    
                    item {
                        Spacer(modifier = Modifier.height(24.dp))
                        Text(
                            text = "TOP 250",
                            style = MaterialTheme.typography.titleLarge,
                            modifier = Modifier.padding(bottom = 8.dp)
                        )
                    }
                    items(top250Movies) { movie ->
                        MovieCard(movie)
                    }
                }
                
                PullRefreshIndicator(
                    refreshing = isLoading,
                    state = pullRefreshState,
                    modifier = Modifier.align(androidx.compose.ui.Alignment.TopCenter)
                )
            }
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun MovieCard(movie: Movie) {
    Card(
        onClick = { /* TODO: Navigate to movie detail */ },
        modifier = Modifier.fillMaxWidth()
    ) {
        Column(
            modifier = Modifier
                .padding(16.dp)
                .fillMaxWidth()
        ) {
            Text(
                text = movie.title,
                style = MaterialTheme.typography.titleMedium,
                color = MaterialTheme.colorScheme.onSurface
            )
            Spacer(modifier = Modifier.height(8.dp))
            Text(
                text = movie.summary,
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant,
                maxLines = 2,
                overflow = TextOverflow.Ellipsis
            )
            Spacer(modifier = Modifier.height(8.dp))
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween
            ) {
                Text(
                    text = "导演: ${movie.directors.joinToString()}",
                    style = MaterialTheme.typography.labelMedium,
                    color = MaterialTheme.colorScheme.primary
                )
                Text(
                    text = "评分: ${movie.rating}",
                    style = MaterialTheme.typography.labelMedium,
                    color = MaterialTheme.colorScheme.outline
                )
            }
        }
    }
}

data class Article(
    val title: String,
    val summary: String,
    val author: String,
    val date: String
)

fun generateSampleArticles(): List<Article> {
    return listOf(
        Article(
            "AI技术在医疗领域的最新应用",
            "人工智能正在彻底改变医疗保健行业，从诊断到治疗计划的制定，AI都发挥着越来越的作用...",
            "张医生",
            "2023-12-20"
        ),
        Article(
            "深度学习框架比较：PyTorch vs TensorFlow",
            "本文将深入比较两大主流深度学习框架的优势与不足，帮助开发者选择最适合的工具...",
            "李工程师",
            "2023-12-19"
        ),
        Article(
            "ChatGPT：改变人机交互的新范式",
            "大型语言模型的出现正在重新定义人类与机器的交互方式，本文探讨了其潜在影响...",
            "王研究员",
            "2023-12-18"
        )
    )
}