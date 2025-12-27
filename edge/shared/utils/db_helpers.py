"""
Database helper utilities for edge services
"""

import sqlite3
import asyncio
import aiosqlite
from typing import Optional, List, Dict, Any, Tuple
from pathlib import Path
import logging

logger = logging.getLogger(__name__)


class DatabaseManager:
    """SQLite database manager for edge services"""
    
    def __init__(self, db_path: str):
        """
        Initialize database manager
        
        Args:
            db_path: Path to SQLite database file
        """
        self.db_path = db_path
        Path(db_path).parent.mkdir(parents=True, exist_ok=True)
        
    async def execute(self, query: str, params: Optional[Tuple] = None) -> None:
        """
        Execute a query (INSERT, UPDATE, DELETE)
        
        Args:
            query: SQL query string
            params: Optional query parameters
        """
        async with aiosqlite.connect(self.db_path) as db:
            await db.execute(query, params or ())
            await db.commit()
            
    async def fetchone(self, query: str, params: Optional[Tuple] = None) -> Optional[Dict[str, Any]]:
        """
        Fetch a single row
        
        Args:
            query: SQL query string
            params: Optional query parameters
            
        Returns:
            Dictionary of row data or None
        """
        async with aiosqlite.connect(self.db_path) as db:
            db.row_factory = aiosqlite.Row
            async with db.execute(query, params or ()) as cursor:
                row = await cursor.fetchone()
                return dict(row) if row else None
                
    async def fetchall(self, query: str, params: Optional[Tuple] = None) -> List[Dict[str, Any]]:
        """
        Fetch all rows
        
        Args:
            query: SQL query string
            params: Optional query parameters
            
        Returns:
            List of dictionaries containing row data
        """
        async with aiosqlite.connect(self.db_path) as db:
            db.row_factory = aiosqlite.Row
            async with db.execute(query, params or ()) as cursor:
                rows = await cursor.fetchall()
                return [dict(row) for row in rows]
                
    async def executemany(self, query: str, params_list: List[Tuple]) -> None:
        """
        Execute a query multiple times with different parameters
        
        Args:
            query: SQL query string
            params_list: List of parameter tuples
        """
        async with aiosqlite.connect(self.db_path) as db:
            await db.executemany(query, params_list)
            await db.commit()
            
    async def create_table(self, table_name: str, schema: str) -> None:
        """
        Create a table if it doesn't exist
        
        Args:
            table_name: Name of the table
            schema: SQL schema definition
        """
        query = f"CREATE TABLE IF NOT EXISTS {table_name} ({schema})"
        await self.execute(query)
        logger.info(f"Table '{table_name}' created or already exists")
        
    async def get_count(self, table_name: str, where: str = "") -> int:
        """
        Get row count from a table
        
        Args:
            table_name: Name of the table
            where: Optional WHERE clause
            
        Returns:
            Number of rows
        """
        query = f"SELECT COUNT(*) as count FROM {table_name}"
        if where:
            query += f" WHERE {where}"
        result = await self.fetchone(query)
        return result['count'] if result else 0
        
    def execute_sync(self, query: str, params: Optional[Tuple] = None) -> None:
        """
        Synchronous execute (for non-async contexts)
        
        Args:
            query: SQL query string
            params: Optional query parameters
        """
        with sqlite3.connect(self.db_path) as conn:
            conn.execute(query, params or ())
            conn.commit()
            
    def fetchall_sync(self, query: str, params: Optional[Tuple] = None) -> List[Dict[str, Any]]:
        """
        Synchronous fetchall (for non-async contexts)
        
        Args:
            query: SQL query string
            params: Optional query parameters
            
        Returns:
            List of dictionaries containing row data
        """
        with sqlite3.connect(self.db_path) as conn:
            conn.row_factory = sqlite3.Row
            cursor = conn.execute(query, params or ())
            return [dict(row) for row in cursor.fetchall()]
