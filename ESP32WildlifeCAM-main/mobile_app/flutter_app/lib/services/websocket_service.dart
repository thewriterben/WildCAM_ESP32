import 'dart:async';
import 'dart:convert';
import 'package:web_socket_channel/web_socket_channel.dart';
import 'package:web_socket_channel/status.dart' as status;
import '../config/api_config.dart';

class WebSocketService {
  WebSocketChannel? _channel;
  StreamController<Map<String, dynamic>>? _messageController;
  bool _isConnected = false;
  int _reconnectAttempts = 0;
  Timer? _reconnectTimer;
  Timer? _pingTimer;
  
  Stream<Map<String, dynamic>> get messages => _messageController!.stream;
  bool get isConnected => _isConnected;
  
  WebSocketService() {
    _messageController = StreamController<Map<String, dynamic>>.broadcast();
  }
  
  // Connect to WebSocket
  Future<void> connect() async {
    if (_isConnected) {
      print('WebSocket already connected');
      return;
    }
    
    try {
      final wsUrl = ApiConfig.getWsUrl();
      print('Connecting to WebSocket: $wsUrl');
      
      _channel = WebSocketChannel.connect(Uri.parse(wsUrl));
      
      _channel!.stream.listen(
        (data) {
          _handleMessage(data);
          _reconnectAttempts = 0; // Reset on successful message
        },
        onError: (error) {
          print('WebSocket error: $error');
          _handleDisconnection();
        },
        onDone: () {
          print('WebSocket connection closed');
          _handleDisconnection();
        },
      );
      
      _isConnected = true;
      _startPingTimer();
      
      print('WebSocket connected successfully');
    } catch (e) {
      print('Failed to connect WebSocket: $e');
      _scheduleReconnect();
    }
  }
  
  // Disconnect WebSocket
  void disconnect() {
    _reconnectTimer?.cancel();
    _pingTimer?.cancel();
    _channel?.sink.close(status.normalClosure);
    _isConnected = false;
    print('WebSocket disconnected');
  }
  
  // Send message
  void send(Map<String, dynamic> message) {
    if (!_isConnected) {
      print('Cannot send message: not connected');
      return;
    }
    
    try {
      final jsonString = json.encode(message);
      _channel?.sink.add(jsonString);
    } catch (e) {
      print('Failed to send message: $e');
    }
  }
  
  // Handle incoming message
  void _handleMessage(dynamic data) {
    try {
      final Map<String, dynamic> message = json.decode(data);
      _messageController?.add(message);
      
      // Handle specific message types
      final type = message['type'];
      switch (type) {
        case 'pong':
          // Pong response received
          break;
        case 'detection':
          print('Wildlife detection: ${message['species']}');
          break;
        case 'status':
          print('Status update received');
          break;
        case 'alert':
          print('Alert: ${message['message']}');
          break;
        default:
          print('Unknown message type: $type');
      }
    } catch (e) {
      print('Failed to handle message: $e');
    }
  }
  
  // Handle disconnection
  void _handleDisconnection() {
    _isConnected = false;
    _pingTimer?.cancel();
    
    if (_reconnectAttempts < ApiConfig.maxReconnectAttempts) {
      _scheduleReconnect();
    } else {
      print('Max reconnect attempts reached');
      _messageController?.addError('Connection lost');
    }
  }
  
  // Schedule reconnection
  void _scheduleReconnect() {
    _reconnectAttempts++;
    print('Scheduling reconnect attempt $_reconnectAttempts');
    
    _reconnectTimer = Timer(ApiConfig.reconnectDelay, () {
      connect();
    });
  }
  
  // Start ping timer to keep connection alive
  void _startPingTimer() {
    _pingTimer = Timer.periodic(Duration(seconds: 30), (timer) {
      if (_isConnected) {
        send({'type': 'ping', 'timestamp': DateTime.now().millisecondsSinceEpoch});
      }
    });
  }
  
  // Dispose resources
  void dispose() {
    disconnect();
    _messageController?.close();
  }
}
