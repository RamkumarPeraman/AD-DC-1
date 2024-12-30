package com.example.act_dir.client_servlet;

import jakarta.servlet.ServletException;
import jakarta.servlet.http.HttpServlet;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;
import javax.json.*;
import java.io.IOException;
import java.io.PrintWriter;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import com.example.act_dir.db.DBConnection;

public class ComputerServlet extends HttpServlet {
    @Override
    protected void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        // Set CORS headers
        response.setHeader("Access-Control-Allow-Origin", "*");
        response.setHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        response.setHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");

        // Handle preflight requests
        if ("OPTIONS".equalsIgnoreCase(request.getMethod())) {
            response.setStatus(HttpServletResponse.SC_OK);
            return;
        }

        response.setContentType("application/json");

        String search = request.getParameter("search");
        String sortBy = request.getParameter("sortBy");

        try (PrintWriter out = response.getWriter()) {
            String jsonData = getComputerNamesAsJson(search, sortBy);
            out.write(jsonData);
        }
    }
    private String getComputerNamesAsJson(String search, String sortBy) {
        StringBuilder jsonData = new StringBuilder("{ \"computers\": [");
        String baseQuery = "SELECT name FROM act WHERE type = 'Computer' AND isDeleted = 'NO'";
        String countQuery = "SELECT COUNT(*) as total_count FROM act WHERE type = 'Computer' AND isDeleted = 'NO'";

        if (search != null && !search.isEmpty()) {
            baseQuery += " AND name LIKE ?";
            countQuery += " AND name LIKE ?";
        }

        if ("asc-desc".equalsIgnoreCase(sortBy)) {
            baseQuery += " ORDER BY name ASC";
        } else if ("desc-asc".equalsIgnoreCase(sortBy)) {
            baseQuery += " ORDER BY name DESC";
        } else if ("new-old".equalsIgnoreCase(sortBy)) {
            baseQuery += " ORDER BY updated_time DESC";
        } else if ("old-new".equalsIgnoreCase(sortBy)) {
            baseQuery += " ORDER BY updated_time ASC";
        }

        int totalCount = 0;
        int resultCount = 0;

        try (Connection conn = DBConnection.getConnection();
             PreparedStatement countStmt = conn.prepareStatement(countQuery);
             PreparedStatement baseStmt = conn.prepareStatement(baseQuery)) {

            if (search != null && !search.isEmpty()) {
                countStmt.setString(1, "%" + search + "%");
                baseStmt.setString(1, "%" + search + "%");
            }

            try (ResultSet countRs = countStmt.executeQuery()) {
                if (countRs.next()) {
                    totalCount = countRs.getInt("total_count");
                }
            }

            try (ResultSet rs = baseStmt.executeQuery()) {
                while (rs.next()) {
                    resultCount++;
                    jsonData.append("{\"name\":\"").append(rs.getString("name")).append("\"},");
                }
                if (jsonData.length() > 13) {
                    jsonData.setLength(jsonData.length() - 1);
                }
            }
        } catch (SQLException e) {
            e.printStackTrace();
            return "{\"error\":\"Database error\"}";
        }

        jsonData.append("], \"totalCount\": ").append(search != null && !search.isEmpty() ? resultCount : totalCount).append("}");
        return jsonData.toString();
    }
}