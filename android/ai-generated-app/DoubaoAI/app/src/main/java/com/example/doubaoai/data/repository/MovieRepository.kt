package com.example.doubaoai.data.repository

import com.example.doubaoai.data.model.Movie
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import org.json.JSONArray
import org.json.JSONObject
import java.net.URL
import javax.inject.Inject

class MovieRepository @Inject constructor() {
    private val baseUrl = "http://localhost:3000/api/movies"

    private suspend fun fetchMovies(endpoint: String): List<Movie> = withContext(Dispatchers.IO) {
        val url = URL("$baseUrl/$endpoint")
        val response = url.readText()
        val jsonArray = JSONArray(response)
        val movies = mutableListOf<Movie>()

        for (i in 0 until jsonArray.length()) {
            val jsonObject = jsonArray.getJSONObject(i)
            movies.add(
                Movie(
                    id = try { jsonObject.getString("id") } catch (e: Exception) { "" },
                    title = try { jsonObject.getString("title") } catch (e: Exception) { "" },
                    originalTitle = try { jsonObject.getString("originalTitle") } catch (e: Exception) { "" },
                    rating = try { jsonObject.getDouble("rating") } catch (e: Exception) { 0.0 },
                    year = try { jsonObject.getString("year") } catch (e: Exception) { "" },
                    genres = try {
                        jsonObject.getJSONArray("genres").let { array ->
                            List(array.length()) { array.getString(it) }
                        }
                    } catch (e: Exception) { emptyList() },
                    directors = try {
                        jsonObject.getJSONArray("directors").let { array ->
                            List(array.length()) { array.getString(it) }
                        }
                    } catch (e: Exception) { emptyList() },
                    casts = try {
                        jsonObject.getJSONArray("casts").let { array ->
                            List(array.length()) { array.getString(it) }
                        }
                    } catch (e: Exception) { emptyList() },
                    image = try { jsonObject.getString("image") } catch (e: Exception) { "" },
                    summary = try { jsonObject.getString("summary") } catch (e: Exception) { "" }
                )
            )
        }
        movies
    }

    suspend fun searchMovies(query: String, start: Int = 0, count: Int = 20): List<Movie> =
        fetchMovies("in_theaters") // 暂时返回正在热映的电影作为搜索结果

    suspend fun getInTheaters(start: Int = 0, count: Int = 20): List<Movie> =
        fetchMovies("in_theaters")

    suspend fun getTop250Movies(start: Int = 0, count: Int = 20): List<Movie> =
        fetchMovies("top250")
}