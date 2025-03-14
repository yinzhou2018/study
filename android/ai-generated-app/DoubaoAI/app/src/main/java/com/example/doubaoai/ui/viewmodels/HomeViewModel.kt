package com.example.doubaoai.ui.viewmodels

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.example.doubaoai.data.model.Movie
import com.example.doubaoai.data.repository.MovieRepository
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch
import javax.inject.Inject

@HiltViewModel
class HomeViewModel @Inject constructor(
    private val movieRepository: MovieRepository
) : ViewModel() {
    private val _inTheatersMovies = MutableStateFlow<List<Movie>>(emptyList())
    val inTheatersMovies: StateFlow<List<Movie>> = _inTheatersMovies.asStateFlow()

    private val _top250Movies = MutableStateFlow<List<Movie>>(emptyList())
    val top250Movies: StateFlow<List<Movie>> = _top250Movies.asStateFlow()

    private val _isLoading = MutableStateFlow(false)
    val isLoading: StateFlow<Boolean> = _isLoading.asStateFlow()

    private val _error = MutableStateFlow<String?>(null)
    val error: StateFlow<String?> = _error.asStateFlow()

    init {
        loadMovies()
    }

    private fun loadMovies() {
        viewModelScope.launch {
            try {
                _isLoading.value = true
                _error.value = null

                val inTheaters = movieRepository.getInTheaters()
                _inTheatersMovies.value = inTheaters

                val top250 = movieRepository.getTop250Movies()
                _top250Movies.value = top250
            } catch (e: Exception) {
                _error.value = e.message ?: "未知错误"
            } finally {
                _isLoading.value = false
            }
        }
    }

    fun refresh() {
        loadMovies()
    }
}