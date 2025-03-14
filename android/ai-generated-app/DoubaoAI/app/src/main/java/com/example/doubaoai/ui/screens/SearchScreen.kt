package com.example.doubaoai.ui.screens

import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Search
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.google.accompanist.systemuicontroller.rememberSystemUiController

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SearchScreen() {
    var searchQuery by remember { mutableStateOf("") }
    var searchResults by remember { mutableStateOf(emptyList<SearchResult>()) }

    val systemUiController = rememberSystemUiController()
    val containerColor = MaterialTheme.colorScheme.primaryContainer
    
    SideEffect {
        systemUiController.setStatusBarColor(containerColor)
    }

    Scaffold(
        topBar = {
            SearchBar(
                query = searchQuery,
                onQueryChange = { query ->
                    searchQuery = query
                    // TODO: Implement actual search logic
                    searchResults = if (query.isNotEmpty()) {
                        generateSampleSearchResults()
                    } else {
                        emptyList()
                    }
                },
                onSearch = { /* TODO: Handle search submission */ },
                active = false,
                onActiveChange = {},
                leadingIcon = { Icon(Icons.Default.Search, contentDescription = "搜索") },
                placeholder = { Text("搜索文章、作者等") },
                modifier = Modifier.fillMaxWidth().padding(16.dp)
            ) {}
        }
    ) { paddingValues ->
        LazyColumn(
            modifier = Modifier
                .fillMaxSize()
                .padding(paddingValues),
            contentPadding = PaddingValues(16.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            items(searchResults) { result ->
                SearchResultItem(result)
            }
        }
    }
}

@Composable
fun SearchResultItem(result: SearchResult) {
    Card(
        modifier = Modifier.fillMaxWidth(),
        colors = CardDefaults.cardColors(
            containerColor = MaterialTheme.colorScheme.surfaceVariant
        )
    ) {
        Column(
            modifier = Modifier
                .padding(16.dp)
                .fillMaxWidth()
        ) {
            Text(
                text = result.title,
                style = MaterialTheme.typography.titleMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
            Spacer(modifier = Modifier.height(4.dp))
            Text(
                text = result.description,
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant.copy(alpha = 0.7f)
            )
        }
    }
}

data class SearchResult(
    val title: String,
    val description: String
)

fun generateSampleSearchResults(): List<SearchResult> {
    return listOf(
        SearchResult(
            "深度学习入门指南",
            "一份全面的深度学习入门教程，包含理论基础和实践案例"
        ),
        SearchResult(
            "机器学习算法比较",
            "详细对比各种机器学习算法的优劣势和适用场景"
        ),
        SearchResult(
            "AI在自然语言处理中的应用",
            "探讨AI技术在NLP领域的最新进展和实际应用"
        )
    )
}