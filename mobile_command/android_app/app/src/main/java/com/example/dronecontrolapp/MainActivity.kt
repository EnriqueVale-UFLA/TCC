package com.example.dronecontrolapp

import android.os.Bundle
import android.webkit.WebView
import android.webkit.WebViewClient
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.gestures.detectDragGestures
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.IntOffset
import androidx.compose.ui.unit.dp
import androidx.compose.ui.viewinterop.AndroidView
import com.example.dronecontrolapp.ui.theme.DroneControlAppTheme
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import okhttp3.OkHttpClient
import okhttp3.Request
import okhttp3.RequestBody.Companion.toRequestBody
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale
import kotlin.math.sqrt

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()

        setContent {
            DroneControlAppTheme {
                DroneControlScreen()
            }
        }
    }
}

@Composable
fun DroneControlScreen() {
    var apiUrl by remember { mutableStateOf("http://192.168.1.166:8000") }
    var value by remember { mutableStateOf("1") }
    var log by remember { mutableStateOf("Aguardando comando...") }
    var currentPage by remember { mutableStateOf(0) }

    val scope = rememberCoroutineScope()
    val client = remember { OkHttpClient() }

    fun addLog(message: String) {
        val time = SimpleDateFormat("HH:mm:ss", Locale.getDefault()).format(Date())
        log = "[$time] $message\n$log"
    }

    fun sendCommand(endpoint: String, label: String) {
        scope.launch(Dispatchers.IO) {
            try {
                val cleanUrl = apiUrl.trim().removeSuffix("/")
                val request = Request.Builder()
                    .url("$cleanUrl$endpoint")
                    .post(ByteArray(0).toRequestBody())
                    .build()

                val response = client.newCall(request).execute()

                if (response.isSuccessful) {
                    addLog("✅ Comando enviado: $label")
                } else {
                    addLog("❌ Erro ${response.code}: $label")
                }

                response.close()
            } catch (e: Exception) {
                addLog("❌ Falha: ${e.message}")
            }
        }
    }

    fun currentValue(): String {
        return value.ifBlank { "1" }
    }

    fun sendMove(direction: String) {
        val meters = currentValue()
        sendCommand("/move/$direction?value=$meters", "${direction.uppercase()} ${meters}m")
    }

    fun sendVelocity(vx: Float, vy: Float, vz: Float, yaw: Float) {
        sendCommand(
            "/velocity?vx=$vx&vy=$vy&vz=$vz&yaw=$yaw",
            "VEL vx=$vx vy=$vy vz=$vz yaw=$yaw"
        )
    }

    Scaffold(
        modifier = Modifier.fillMaxSize()
    ) { innerPadding ->

        Column(
            modifier = Modifier
                .padding(innerPadding)
                .padding(16.dp)
                .verticalScroll(rememberScrollState()),
            verticalArrangement = Arrangement.spacedBy(16.dp)
        ) {
            Text(
                text = "Drone Control App",
                style = MaterialTheme.typography.headlineMedium,
                fontWeight = FontWeight.Bold
            )

            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                Button(
                    onClick = { currentPage = 0 },
                    modifier = Modifier.weight(1f)
                ) {
                    Text("Buttons")
                }

                Button(
                    onClick = { currentPage = 1 },
                    modifier = Modifier.weight(1f)
                ) {
                    Text("Joystick")
                }
            }

            OutlinedTextField(
                value = apiUrl,
                onValueChange = { apiUrl = it },
                label = { Text("URL da API") },
                modifier = Modifier.fillMaxWidth(),
                singleLine = true
            )

            if (currentPage == 0) {
                ButtonsPage(
                    apiUrl = apiUrl,
                    value = value,
                    onValueChange = { value = it },
                    log = log,
                    sendCommand = ::sendCommand,
                    sendMove = ::sendMove,
                    currentValue = ::currentValue
                )
            } else {
                JoystickPage(
                    apiUrl = apiUrl,
                    sendVelocity = ::sendVelocity,
                    sendCommand = ::sendCommand
                )
            }
        }
    }
}

@Composable
fun ButtonsPage(
    apiUrl: String,
    value: String,
    onValueChange: (String) -> Unit,
    log: String,
    sendCommand: (String, String) -> Unit,
    sendMove: (String) -> Unit,
    currentValue: () -> String
) {
    OutlinedTextField(
        value = value,
        onValueChange = onValueChange,
        label = { Text("Distância / Altitude (m)") },
        modifier = Modifier.fillMaxWidth(),
        singleLine = true
    )

    CameraCard(apiUrl)

    ElevatedCard(modifier = Modifier.fillMaxWidth()) {
        Column(
            modifier = Modifier.padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            Text(
                text = "Comandos",
                style = MaterialTheme.typography.titleLarge,
                fontWeight = FontWeight.Bold
            )

            Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                Button(
                    onClick = { sendCommand("/arm", "ARM") },
                    modifier = Modifier.weight(1f)
                ) {
                    Text("ARM")
                }

                Button(
                    onClick = { sendCommand("/offboard", "OFFBOARD") },
                    modifier = Modifier.weight(1f)
                ) {
                    Text("OFFBOARD")
                }
            }

            Button(
                onClick = {
                    val meters = currentValue()
                    sendCommand("/takeoff?value=$meters", "TAKEOFF ${meters}m")
                },
                modifier = Modifier.fillMaxWidth()
            ) {
                Text("TAKEOFF")
            }

            Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                Button(onClick = { sendMove("left") }, modifier = Modifier.weight(1f)) { Text("LEFT") }
                Button(onClick = { sendMove("up") }, modifier = Modifier.weight(1f)) { Text("UP") }
                Button(onClick = { sendMove("right") }, modifier = Modifier.weight(1f)) { Text("RIGHT") }
            }

            Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                Button(onClick = { sendMove("back") }, modifier = Modifier.weight(1f)) { Text("BACK") }
                Button(onClick = { sendMove("down") }, modifier = Modifier.weight(1f)) { Text("DOWN") }
                Button(onClick = { sendMove("forward") }, modifier = Modifier.weight(1f)) { Text("FORWARD") }
            }

            Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                Button(
                    onClick = { sendCommand("/aruco/on", "ARUCO ON") },
                    modifier = Modifier.weight(1f)
                ) {
                    Text("ARUCO ON")
                }

                Button(
                    onClick = { sendCommand("/aruco/off", "ARUCO OFF") },
                    modifier = Modifier.weight(1f)
                ) {
                    Text("ARUCO OFF")
                }
            }

            Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                Button(
                    onClick = { sendCommand("/person/on", "PERSON ON") },
                    modifier = Modifier.weight(1f)
                ) {
                    Text("PERSON ON")
                }

                Button(
                    onClick = { sendCommand("/person/off", "PERSON OFF") },
                    modifier = Modifier.weight(1f)
                ) {
                    Text("PERSON OFF")
                }
            }

            Button(
                onClick = { sendCommand("/land", "LAND") },
                modifier = Modifier.fillMaxWidth(),
                colors = ButtonDefaults.buttonColors(containerColor = MaterialTheme.colorScheme.error)
            ) {
                Text("LAND")
            }

            Button(
                onClick = { sendCommand("/rtl", "RTL") },
                modifier = Modifier.fillMaxWidth(),
                colors = ButtonDefaults.buttonColors(containerColor = MaterialTheme.colorScheme.error)
            ) {
                Text("RTL")
            }
        }
    }

    ElevatedCard(modifier = Modifier.fillMaxWidth()) {
        Column(
            modifier = Modifier.padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            Text(
                text = "Log",
                style = MaterialTheme.typography.titleLarge,
                fontWeight = FontWeight.Bold
            )

            Text(text = log)
        }
    }
}

@Composable
fun JoystickPage(
    apiUrl: String,
    sendVelocity: (Float, Float, Float, Float) -> Unit,
    sendCommand: (String, String) -> Unit
) {
    CameraCard(apiUrl)

    Text(
        text = "Joystick Control",
        style = MaterialTheme.typography.titleLarge,
        fontWeight = FontWeight.Bold
    )

    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.SpaceEvenly
    ) {
        Joystick(label = "ALT / YAW") { x, y ->
            val yaw = x * 3.0f
            val vz = y
            sendVelocity(0f, 0f, vz, yaw)
        }

        Joystick(label = "MOVE") { x, y ->
            val vx = -y
            val vy = x
            sendVelocity(vx, vy, 0f, 0f)
        }
    }

    Button(
        onClick = { sendCommand("/stop", "STOP") },
        modifier = Modifier.fillMaxWidth(),
        colors = ButtonDefaults.buttonColors(containerColor = MaterialTheme.colorScheme.error)
    ) {
        Text("STOP")
    }
}

@Composable
fun CameraCard(apiUrl: String) {
    ElevatedCard(modifier = Modifier.fillMaxWidth()) {
        Column(
            modifier = Modifier.padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(16.dp)
        ) {
            Text(
                text = "Cameras",
                style = MaterialTheme.typography.titleLarge,
                fontWeight = FontWeight.Bold
            )

            Text(
                text = "Camera Down",
                style = MaterialTheme.typography.titleMedium,
                fontWeight = FontWeight.Bold
            )

            CameraWebView(
                url = "${apiUrl.trim().removeSuffix("/")}/video_feed_down"
            )

            Text(
                text = "Camera Front",
                style = MaterialTheme.typography.titleMedium,
                fontWeight = FontWeight.Bold
            )

            CameraWebView(
                url = "${apiUrl.trim().removeSuffix("/")}/video_feed_front"
            )
        }
    }
}

@Composable
fun CameraWebView(url: String) {
    AndroidView(
        factory = { context ->
            WebView(context).apply {
                webViewClient = WebViewClient()
                settings.javaScriptEnabled = true
                settings.loadWithOverviewMode = true
                settings.useWideViewPort = true
                loadUrl(url)
            }
        },
        update = { webView ->
            webView.loadUrl(url)
        },
        modifier = Modifier
            .fillMaxWidth()
            .height(220.dp)
    )
}

@Composable
fun Joystick(
    label: String,
    onMove: (Float, Float) -> Unit
) {
    var offset by remember { mutableStateOf(Offset.Zero) }

    val size = 160.dp
    val knobSize = 60.dp

    val sizePx = with(LocalDensity.current) { size.toPx() }
    val radius = sizePx / 2.5f

    Column(horizontalAlignment = Alignment.CenterHorizontally) {
        Text(label)

        Box(
            modifier = Modifier
                .size(size)
                .background(Color.DarkGray, CircleShape)
                .border(2.dp, Color.Gray, CircleShape)
                .pointerInput(Unit) {
                    detectDragGestures(
                        onDragEnd = {
                            offset = Offset.Zero
                            onMove(0f, 0f)
                        },
                        onDragCancel = {
                            offset = Offset.Zero
                            onMove(0f, 0f)
                        }
                    ) { change, dragAmount ->
                        change.consume()

                        val newOffset = offset + Offset(dragAmount.x, dragAmount.y)

                        val distance = sqrt(
                            newOffset.x * newOffset.x +
                                    newOffset.y * newOffset.y
                        )

                        offset =
                            if (distance <= radius) {
                                newOffset
                            } else {
                                val scale = radius / distance
                                Offset(newOffset.x * scale, newOffset.y * scale)
                            }

                        val normalizedX = (offset.x / radius).coerceIn(-1f, 1f)
                        val normalizedY = (offset.y / radius).coerceIn(-1f, 1f)

                        onMove(normalizedX, normalizedY)
                    }
                },
            contentAlignment = Alignment.Center
        ) {
            Box(
                modifier = Modifier
                    .offset {
                        IntOffset(offset.x.toInt(), offset.y.toInt())
                    }
                    .size(knobSize)
                    .background(Color.LightGray, CircleShape)
            )
        }
    }
}