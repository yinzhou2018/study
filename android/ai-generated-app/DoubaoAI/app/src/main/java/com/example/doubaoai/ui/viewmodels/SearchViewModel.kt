package com.example.doubaoai.ui.viewmodels

import androidx.lifecycle.ViewModel
import com.example.doubaoai.data.repository.MovieRepository
import dagger.hilt.android.lifecycle.HiltViewModel
import javax.inject.Inject

@HiltViewModel
class SearchViewModel @Inject constructor(
    private val movieRepository: MovieRepository
) : ViewModel()