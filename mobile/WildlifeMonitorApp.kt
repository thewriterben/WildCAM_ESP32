/**
 * Android app for field researchers
 * Real-time monitoring and configuration
 */

package com.wildlife.monitor

import android.Manifest
import android.bluetooth.BluetoothAdapter
import android.location.Location
import androidx.compose.runtime.*
import androidx.lifecycle.ViewModel
import kotlinx.coroutines.flow.MutableStateFlow
import retrofit2.Retrofit
import retrofit2.http.*
import java.util.Date
import com.wildlife.monitor.ar.*

// Data Models
data class Camera(
    val id: String,
    val name: String,
    val location: LatLng,
    val batteryLevel: Float,
    val lastSeen: Date,
    val status: CameraStatus
)

data class Detection(
    val id: String,
    val cameraId: String,
    val timestamp: Date,
    val species: String,
    val confidence: Float,
    val imageUrl: String,
    val verified: Boolean = false
)

enum class CameraStatus {
    ONLINE, OFFLINE, LOW_BATTERY, ERROR
}

// API Service
interface WildlifeAPI {
    @GET("cameras")
    suspend fun getCameras(): List<Camera>
    
    @GET("detections")
    suspend fun getDetections(
        @Query("start") startDate: Date,
        @Query("end") endDate: Date
    ): List<Detection>
    
    @POST("detections/{id}/verify")
    suspend fun verifyDetection(
        @Path("id") detectionId: String,
        @Body verification: VerificationData
    )
    
    @Multipart
    @POST("observations")
    suspend fun submitObservation(
        @Part image: MultipartBody.Part,
        @Part("metadata") metadata: RequestBody
    )
}

// Main ViewModel
class WildlifeViewModel : ViewModel() {
    private val api = Retrofit.Builder()
        .baseUrl("https://api.wildlife-monitor.com/")
        .build()
        .create(WildlifeAPI::class.java)
    
    val cameras = MutableStateFlow<List<Camera>>(emptyList())
    val detections = MutableStateFlow<List<Detection>>(emptyList())
    val selectedCamera = MutableStateFlow<Camera?>(null)
    
    fun refreshCameras() {
        viewModelScope.launch {
            cameras.value = api.getCameras()
        }
    }
    
    fun verifyDetection(detection: Detection, actualSpecies: String) {
        viewModelScope.launch {
            api.verifyDetection(
                detection.id,
                VerificationData(
                    actualSpecies = actualSpecies,
                    verifiedBy = getCurrentUser(),
                    confidence = 1.0f
                )
            )
        }
    }
    
    fun submitFieldObservation(
        image: ByteArray,
        location: Location,
        species: String,
        notes: String
    ) {
        viewModelScope.launch {
            val metadata = FieldObservation(
                location = LatLng(location.latitude, location.longitude),
                species = species,
                notes = notes,
                timestamp = Date(),
                observer = getCurrentUser()
            )
            
            api.submitObservation(
                MultipartBody.Part.createFormData(
                    "image",
                    "observation.jpg",
                    image.toRequestBody()
                ),
                metadata.toJson().toRequestBody()
            )
        }
    }
}

// UI Screens
@Composable
fun WildlifeMonitorApp(viewModel: WildlifeViewModel) {
    val navController = rememberNavController()
    val context = LocalContext.current
    
    // Initialize AR app (lazy)
    val arApp = remember { ARWildlifeApp(context) }
    
    NavHost(navController, startDestination = "dashboard") {
        composable("dashboard") {
            DashboardScreen(viewModel, navController)
        }
        composable("camera/{cameraId}") { backStackEntry ->
            CameraDetailScreen(
                viewModel,
                backStackEntry.arguments?.getString("cameraId") ?: ""
            )
        }
        composable("field_observation") {
            FieldObservationScreen(viewModel)
        }
        composable("species_guide") {
            SpeciesGuideScreen()
        }
        composable("ar_mode") {
            ARWildlifeScreen(viewModel, arApp)
        }
    }
}

@Composable
fun DashboardScreen(
    viewModel: WildlifeViewModel,
    navController: NavController
) {
    val cameras by viewModel.cameras.collectAsState()
    val detections by viewModel.detections.collectAsState()
    
    Column {
        // Map view with camera locations
        GoogleMap(
            modifier = Modifier.height(300.dp),
            cameraPositions = cameras.map { it.location }
        ) { camera ->
            navController.navigate("camera/${camera.id}")
        }
        
        // Recent detections list
        LazyColumn {
            items(detections) { detection ->
                DetectionCard(
                    detection = detection,
                    onVerify = { species ->
                        viewModel.verifyDetection(detection, species)
                    }
                )
            }
        }
        
        // Action buttons row
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceEvenly
        ) {
            // Field observations button
            FloatingActionButton(
                onClick = { navController.navigate("field_observation") }
            ) {
                Icon(Icons.Default.CameraAlt)
            }
            
            // AR Mode button
            FloatingActionButton(
                onClick = { navController.navigate("ar_mode") },
                containerColor = MaterialTheme.colorScheme.secondary
            ) {
                Icon(Icons.Default.ViewInAr, contentDescription = "AR Mode")
            }
        }
    }
}