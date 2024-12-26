package com.example.act_dir.db;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;

public class DBConnection {
    public static Connection getConnection() throws SQLException {
        try {
            Class.forName("com.mysql.cj.jdbc.Driver");
            return DriverManager.getConnection(
                    "jdbc:mysql://localhost:3306/act_dir",
                    "ram",
                    "Dudububu@27"
            );
        } catch (Exception e) {
            e.printStackTrace();
            throw new SQLException("Database connection error");
        }
    }
}
