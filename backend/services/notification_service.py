"""
Multi-Channel Notification Service
Handles alert delivery via email, SMS, push notifications, webhooks
"""

import logging
import json
from datetime import datetime, timedelta
from typing import Dict, List, Optional
import smtplib
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart
import requests
from collections import defaultdict, deque

logger = logging.getLogger(__name__)


class NotificationChannel:
    """Notification channel types"""
    EMAIL = 'email'
    SMS = 'sms'
    PUSH = 'push'
    WEBHOOK = 'webhook'
    SLACK = 'slack'
    DISCORD = 'discord'


class NotificationService:
    """
    Multi-channel notification delivery with rate limiting and escalation
    """
    
    def __init__(self, config: Optional[Dict] = None):
        self.config = config or {}
        
        # Rate limiting
        self.rate_limits = defaultdict(lambda: {
            'count': 0,
            'window_start': datetime.utcnow(),
            'max_per_hour': 50
        })
        
        # Batch notifications
        self.batch_queue = defaultdict(list)
        self.batch_interval = timedelta(minutes=5)
        self.last_batch_send = datetime.utcnow()
        
        # Notification history for deduplication
        self.recent_notifications = deque(maxlen=1000)
        
    def send_alert(
        self,
        alert_data: Dict,
        channels: List[str],
        recipients: List[Dict],
        priority: int = 100
    ) -> Dict:
        """
        Send alert through specified channels
        
        Args:
            alert_data: Alert information
            channels: List of channels to use
            recipients: List of recipient configurations
            priority: Alert priority (higher = more urgent)
            
        Returns:
            Delivery status for each channel
        """
        results = {}
        
        # Check for duplicate alerts
        if self._is_duplicate(alert_data):
            logger.info("Skipping duplicate alert")
            return {'status': 'skipped', 'reason': 'duplicate'}
        
        # Check rate limits
        if not self._check_rate_limit(alert_data.get('camera_id', 'default')):
            if priority < 500:  # Only batch non-critical alerts
                self._add_to_batch(alert_data, channels, recipients)
                return {'status': 'batched', 'reason': 'rate_limit'}
        
        # Send through each channel
        for channel in channels:
            try:
                if channel == NotificationChannel.EMAIL:
                    results[channel] = self._send_email(alert_data, recipients)
                elif channel == NotificationChannel.WEBHOOK:
                    results[channel] = self._send_webhook(alert_data, recipients)
                elif channel == NotificationChannel.SLACK:
                    results[channel] = self._send_slack(alert_data, recipients)
                elif channel == NotificationChannel.DISCORD:
                    results[channel] = self._send_discord(alert_data, recipients)
            except Exception as e:
                logger.error(f"Error sending notification via {channel}: {e}")
                results[channel] = {'success': False, 'error': str(e)}
        
        # Track notification
        self._track_notification(alert_data)
        
        return results
    
    def _send_email(self, alert_data: Dict, recipients: List[Dict]) -> Dict:
        """Send email notification"""
        try:
            smtp_config = self.config.get('smtp', {})
            if not smtp_config:
                return {'success': False, 'error': 'SMTP not configured'}
            
            # Create message
            msg = MIMEMultipart('alternative')
            msg['Subject'] = self._format_subject(alert_data)
            msg['From'] = smtp_config.get('from_address', 'noreply@wildcam.io')
            
            # Get recipient emails
            to_emails = [r.get('email') for r in recipients if r.get('email')]
            if not to_emails:
                return {'success': False, 'error': 'No email recipients'}
            
            msg['To'] = ', '.join(to_emails)
            
            # Create HTML and plain text versions
            text_body = self._format_text_body(alert_data)
            html_body = self._format_html_body(alert_data)
            
            msg.attach(MIMEText(text_body, 'plain'))
            msg.attach(MIMEText(html_body, 'html'))
            
            # Send email
            with smtplib.SMTP(smtp_config.get('host'), smtp_config.get('port', 587)) as server:
                server.starttls()
                server.login(smtp_config.get('username'), smtp_config.get('password'))
                server.send_message(msg)
            
            logger.info(f"Email sent to {len(to_emails)} recipients")
            return {'success': True, 'recipients': len(to_emails)}
            
        except Exception as e:
            logger.error(f"Email send error: {e}")
            return {'success': False, 'error': str(e)}
    
    def _send_webhook(self, alert_data: Dict, recipients: List[Dict]) -> Dict:
        """Send webhook notification"""
        try:
            webhook_urls = [r.get('webhook_url') for r in recipients if r.get('webhook_url')]
            if not webhook_urls:
                return {'success': False, 'error': 'No webhook URLs'}
            
            # Prepare webhook payload
            payload = {
                'event': 'wildlife_alert',
                'timestamp': datetime.utcnow().isoformat(),
                'alert': alert_data
            }
            
            sent_count = 0
            for url in webhook_urls:
                try:
                    response = requests.post(
                        url,
                        json=payload,
                        timeout=10,
                        headers={'Content-Type': 'application/json'}
                    )
                    if response.status_code in [200, 201, 202]:
                        sent_count += 1
                except Exception as e:
                    logger.error(f"Webhook error for {url}: {e}")
            
            return {'success': sent_count > 0, 'webhooks': sent_count}
            
        except Exception as e:
            logger.error(f"Webhook send error: {e}")
            return {'success': False, 'error': str(e)}
    
    def _send_slack(self, alert_data: Dict, recipients: List[Dict]) -> Dict:
        """Send Slack notification"""
        try:
            webhook_urls = [r.get('slack_webhook') for r in recipients if r.get('slack_webhook')]
            if not webhook_urls:
                return {'success': False, 'error': 'No Slack webhooks'}
            
            # Format Slack message
            color = self._get_alert_color(alert_data.get('alert_level'))
            
            slack_message = {
                'attachments': [{
                    'color': color,
                    'title': self._format_subject(alert_data),
                    'text': self._format_text_body(alert_data),
                    'fields': [
                        {
                            'title': 'Species',
                            'value': alert_data.get('species', 'Unknown'),
                            'short': True
                        },
                        {
                            'title': 'Confidence',
                            'value': f"{alert_data.get('confidence', 0) * 100:.1f}%",
                            'short': True
                        },
                        {
                            'title': 'Camera',
                            'value': alert_data.get('camera_id', 'Unknown'),
                            'short': True
                        },
                        {
                            'title': 'Time',
                            'value': alert_data.get('timestamp', ''),
                            'short': True
                        }
                    ],
                    'image_url': alert_data.get('image_url'),
                    'footer': 'WildCAM Alert System',
                    'ts': int(datetime.utcnow().timestamp())
                }]
            }
            
            sent_count = 0
            for webhook_url in webhook_urls:
                try:
                    response = requests.post(webhook_url, json=slack_message)
                    if response.status_code == 200:
                        sent_count += 1
                except Exception as e:
                    logger.error(f"Slack send error: {e}")
            
            return {'success': sent_count > 0, 'channels': sent_count}
            
        except Exception as e:
            logger.error(f"Slack send error: {e}")
            return {'success': False, 'error': str(e)}
    
    def _send_discord(self, alert_data: Dict, recipients: List[Dict]) -> Dict:
        """Send Discord notification"""
        try:
            webhook_urls = [r.get('discord_webhook') for r in recipients if r.get('discord_webhook')]
            if not webhook_urls:
                return {'success': False, 'error': 'No Discord webhooks'}
            
            # Format Discord embed
            color_map = {
                'emergency': 0xFF0000,  # Red
                'critical': 0xFF6600,   # Orange
                'warning': 0xFFCC00,    # Yellow
                'info': 0x0099FF        # Blue
            }
            
            discord_message = {
                'embeds': [{
                    'title': self._format_subject(alert_data),
                    'description': self._format_text_body(alert_data),
                    'color': color_map.get(alert_data.get('alert_level', 'info'), 0x0099FF),
                    'fields': [
                        {
                            'name': 'Species',
                            'value': alert_data.get('species', 'Unknown'),
                            'inline': True
                        },
                        {
                            'name': 'Confidence',
                            'value': f"{alert_data.get('confidence', 0) * 100:.1f}%",
                            'inline': True
                        },
                        {
                            'name': 'Camera',
                            'value': alert_data.get('camera_id', 'Unknown'),
                            'inline': True
                        }
                    ],
                    'image': {
                        'url': alert_data.get('image_url', '')
                    } if alert_data.get('image_url') else None,
                    'timestamp': datetime.utcnow().isoformat(),
                    'footer': {
                        'text': 'WildCAM Alert System'
                    }
                }]
            }
            
            sent_count = 0
            for webhook_url in webhook_urls:
                try:
                    response = requests.post(webhook_url, json=discord_message)
                    if response.status_code == 204:
                        sent_count += 1
                except Exception as e:
                    logger.error(f"Discord send error: {e}")
            
            return {'success': sent_count > 0, 'channels': sent_count}
            
        except Exception as e:
            logger.error(f"Discord send error: {e}")
            return {'success': False, 'error': str(e)}
    
    def _format_subject(self, alert_data: Dict) -> str:
        """Format alert subject line"""
        level = alert_data.get('alert_level', 'info').upper()
        species = alert_data.get('species', 'Wildlife')
        camera = alert_data.get('camera_id', 'Unknown')
        
        return f"[{level}] {species} detected at {camera}"
    
    def _format_text_body(self, alert_data: Dict, max_length: Optional[int] = None) -> str:
        """Format plain text alert body"""
        lines = [
            f"Species: {alert_data.get('species', 'Unknown')}",
            f"Confidence: {alert_data.get('confidence', 0) * 100:.1f}%",
            f"Alert Level: {alert_data.get('alert_level', 'info').upper()}",
            f"Camera: {alert_data.get('camera_id', 'Unknown')}",
            f"Location: {alert_data.get('location', 'Unknown')}",
            f"Time: {alert_data.get('timestamp', datetime.utcnow().isoformat())}",
        ]
        
        if alert_data.get('consecutive_detections', 0) > 1:
            lines.append(f"Consecutive detections: {alert_data['consecutive_detections']}")
        
        body = '\n'.join(lines)
        
        if max_length and len(body) > max_length:
            body = body[:max_length-3] + '...'
        
        return body
    
    def _format_html_body(self, alert_data: Dict) -> str:
        """Format HTML alert body"""
        level_colors = {
            'emergency': '#dc3545',
            'critical': '#fd7e14',
            'warning': '#ffc107',
            'info': '#17a2b8'
        }
        color = level_colors.get(alert_data.get('alert_level', 'info'), '#17a2b8')
        
        html = f"""
        <html>
        <body style="font-family: Arial, sans-serif;">
            <div style="background-color: {color}; color: white; padding: 20px; border-radius: 5px;">
                <h2>{self._format_subject(alert_data)}</h2>
            </div>
            <div style="padding: 20px;">
                <p><strong>Species:</strong> {alert_data.get('species', 'Unknown')}</p>
                <p><strong>Confidence:</strong> {alert_data.get('confidence', 0) * 100:.1f}%</p>
                <p><strong>Alert Level:</strong> {alert_data.get('alert_level', 'info').upper()}</p>
                <p><strong>Camera:</strong> {alert_data.get('camera_id', 'Unknown')}</p>
                <p><strong>Location:</strong> {alert_data.get('location', 'Unknown')}</p>
                <p><strong>Time:</strong> {alert_data.get('timestamp', datetime.utcnow().isoformat())}</p>
        """
        
        if alert_data.get('image_url'):
            html += f'<p><img src="{alert_data["image_url"]}" style="max-width: 600px;" /></p>'
        
        html += """
            </div>
        </body>
        </html>
        """
        return html
    
    def _get_alert_color(self, alert_level: str) -> str:
        """Get color for alert level"""
        colors = {
            'emergency': '#dc3545',
            'critical': '#fd7e14',
            'warning': '#ffc107',
            'info': '#17a2b8'
        }
        return colors.get(alert_level, '#17a2b8')
    
    def _check_rate_limit(self, identifier: str) -> bool:
        """Check if rate limit allows sending"""
        limit_data = self.rate_limits[identifier]
        
        # Reset counter if window expired
        if datetime.utcnow() - limit_data['window_start'] > timedelta(hours=1):
            limit_data['count'] = 0
            limit_data['window_start'] = datetime.utcnow()
        
        # Check limit
        if limit_data['count'] >= limit_data['max_per_hour']:
            return False
        
        # Increment counter
        limit_data['count'] += 1
        return True
    
    def _add_to_batch(self, alert_data: Dict, channels: List[str], recipients: List[Dict]):
        """Add alert to batch queue"""
        batch_key = alert_data.get('camera_id', 'default')
        self.batch_queue[batch_key].append({
            'alert': alert_data,
            'channels': channels,
            'recipients': recipients,
            'queued_at': datetime.utcnow()
        })
    
    def _is_duplicate(self, alert_data: Dict) -> bool:
        """Check if alert is duplicate of recent notification"""
        # Create signature for alert
        signature = (
            alert_data.get('species'),
            alert_data.get('camera_id'),
            alert_data.get('alert_level')
        )
        
        # Check if similar alert sent in last 5 minutes
        cutoff = datetime.utcnow() - timedelta(minutes=5)
        for notification in self.recent_notifications:
            if (notification['signature'] == signature and 
                notification['timestamp'] > cutoff):
                return True
        
        return False
    
    def _track_notification(self, alert_data: Dict):
        """Track sent notification for deduplication"""
        signature = (
            alert_data.get('species'),
            alert_data.get('camera_id'),
            alert_data.get('alert_level')
        )
        
        self.recent_notifications.append({
            'signature': signature,
            'timestamp': datetime.utcnow(),
            'alert_id': alert_data.get('id')
        })
