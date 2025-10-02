import 'package:awesome_notifications/awesome_notifications.dart';
import 'package:firebase_messaging/firebase_messaging.dart';

class NotificationService {
  static final NotificationService _instance = NotificationService._internal();
  factory NotificationService() => _instance;
  NotificationService._internal();
  
  final FirebaseMessaging _firebaseMessaging = FirebaseMessaging.instance;
  
  // Initialize notifications
  Future<void> initialize() async {
    // Request permissions for iOS
    NotificationSettings settings = await _firebaseMessaging.requestPermission(
      alert: true,
      announcement: false,
      badge: true,
      carPlay: false,
      criticalAlert: false,
      provisional: false,
      sound: true,
    );
    
    print('User granted notification permission: ${settings.authorizationStatus}');
    
    // Get FCM token
    String? token = await _firebaseMessaging.getToken();
    print('FCM Token: $token');
    
    // Listen to foreground messages
    FirebaseMessaging.onMessage.listen(_handleForegroundMessage);
    
    // Listen to background message taps
    FirebaseMessaging.onMessageOpenedApp.listen(_handleMessageOpenedApp);
    
    // Check if app was opened from notification
    RemoteMessage? initialMessage = await _firebaseMessaging.getInitialMessage();
    if (initialMessage != null) {
      _handleMessageOpenedApp(initialMessage);
    }
  }
  
  // Handle foreground messages
  void _handleForegroundMessage(RemoteMessage message) {
    print('Got a message whilst in the foreground!');
    print('Message data: ${message.data}');
    
    if (message.notification != null) {
      showLocalNotification(
        title: message.notification!.title ?? 'Wildlife Detection',
        body: message.notification!.body ?? 'New wildlife detected',
        payload: message.data,
      );
    }
  }
  
  // Handle notification tap
  void _handleMessageOpenedApp(RemoteMessage message) {
    print('Message clicked! Data: ${message.data}');
    // Navigate to appropriate screen based on message data
  }
  
  // Show local notification
  Future<void> showLocalNotification({
    required String title,
    required String body,
    Map<String, dynamic>? payload,
    String channelKey = 'wildlife_alerts',
  }) async {
    await AwesomeNotifications().createNotification(
      content: NotificationContent(
        id: DateTime.now().millisecondsSinceEpoch.remainder(100000),
        channelKey: channelKey,
        title: title,
        body: body,
        payload: payload?.map((key, value) => MapEntry(key, value.toString())),
        notificationLayout: NotificationLayout.BigPicture,
        bigPicture: 'asset://assets/images/wildlife_notification.png',
        largeIcon: 'asset://assets/images/app_icon.png',
      ),
    );
  }
  
  // Show wildlife detection notification
  Future<void> showWildlifeDetection({
    required String species,
    required double confidence,
    String? imageUrl,
  }) async {
    await showLocalNotification(
      title: 'Wildlife Detected!',
      body: '$species detected with ${(confidence * 100).toStringAsFixed(0)}% confidence',
      payload: {
        'type': 'wildlife_detection',
        'species': species,
        'confidence': confidence.toString(),
        'imageUrl': imageUrl ?? '',
      },
      channelKey: 'wildlife_alerts',
    );
  }
  
  // Show environmental alert
  Future<void> showEnvironmentalAlert({
    required String alertType,
    required String message,
  }) async {
    await showLocalNotification(
      title: 'Environmental Alert',
      body: message,
      payload: {
        'type': 'environmental_alert',
        'alertType': alertType,
      },
      channelKey: 'system_alerts',
    );
  }
  
  // Show system alert
  Future<void> showSystemAlert({
    required String title,
    required String message,
  }) async {
    await showLocalNotification(
      title: title,
      body: message,
      payload: {
        'type': 'system_alert',
      },
      channelKey: 'system_alerts',
    );
  }
  
  // Subscribe to FCM topic
  Future<void> subscribeToTopic(String topic) async {
    await _firebaseMessaging.subscribeToTopic(topic);
    print('Subscribed to topic: $topic');
  }
  
  // Unsubscribe from FCM topic
  Future<void> unsubscribeFromTopic(String topic) async {
    await _firebaseMessaging.unsubscribeFromTopic(topic);
    print('Unsubscribed from topic: $topic');
  }
}
