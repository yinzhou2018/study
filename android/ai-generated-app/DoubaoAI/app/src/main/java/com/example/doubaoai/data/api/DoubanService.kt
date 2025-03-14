package com.example.doubaoai.data.api

import com.example.doubaoai.data.model.Movie
import retrofit2.http.GET
import retrofit2.http.Query

interface DoubanService {
    @GET("v2/movie/search")
    suspend fun searchMovies(
        @Query("q") query: String,
        @Query("start") start: Int = 0,
        @Query("count") count: Int = 20
    ): List<Movie>

    @GET("v2/movie/in_theaters")
    suspend fun getInTheaters(
        @Query("start") start: Int = 0,
        @Query("count") count: Int = 20
    ): List<Movie>

    @GET("v2/movie/top250")
    suspend fun getTop250Movies(
        @Query("start") start: Int = 0,
        @Query("count") count: Int = 20
    ): List<Movie>
}