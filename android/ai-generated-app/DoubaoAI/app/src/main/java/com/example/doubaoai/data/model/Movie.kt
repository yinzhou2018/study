package com.example.doubaoai.data.model

data class Movie(
    val id: String,
    val title: String,
    val year: String,
    val originalTitle: String,
    val rating: Double,
    val directors: List<String>,
    val casts: List<String>,
    val genres: List<String>,
    val image: String,
    val summary: String
)