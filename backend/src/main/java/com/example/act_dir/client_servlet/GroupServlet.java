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

public class GroupServlet extends HttpServlet {

    @Override
    protected void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        // Set CORS headers
        response.setHeader("Access-Control-Allow-Origin", "*");
        response.setHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        response.setHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
        if (request.getMethod().equals("OPTIONS")) {
            response.setStatus(HttpServletResponse.SC_ACCEPTED);
            return;
        }

        String searchQuery = request.getParameter("search");
        String sortBy = request.getParameter("sortBy");

        response.setContentType("application/json");

        try (PrintWriter out = response.getWriter()) {
            String jsonData = getGroupNamesAsJson(searchQuery, sortBy);
            out.write(jsonData);
        }
    }
    private String getGroupNamesAsJson(String searchQuery, String sortBy) {
        StringBuilder jsonData = new StringBuilder("{");
        String query = "SELECT name FROM act WHERE type = 'Group' AND isDeleted = 'NO'";

        if (searchQuery != null && !searchQuery.isEmpty()) {
            query += " AND name LIKE ?";
        }

        if (sortBy != null && !sortBy.isEmpty()) {
            switch (sortBy) {
                case "asc-desc":
                    query += " ORDER BY name ASC";
                    break;
                case "desc-asc":
                    query += " ORDER BY name DESC";
                    break;
                case "new-old":
                    query += " ORDER BY whenCreated DESC";
                    break;
                case "old-new":
                    query += " ORDER BY whenCreated ASC";
                    break;
            }
        }
        try (Connection conn = DBConnection.getConnection();
             PreparedStatement pstmt = conn.prepareStatement(query)) {
            if (searchQuery != null && !searchQuery.isEmpty()) {
                pstmt.setString(1, "%" + searchQuery + "%");
            }
            try (ResultSet rs = pstmt.executeQuery()) {
                StringBuilder groupsJson = new StringBuilder("[");
                while (rs.next()) {
                    groupsJson.append("{\"name\":\"").append(rs.getString("name")).append("\"},");
                }
                if (groupsJson.length() > 1) {
                    groupsJson.setLength(groupsJson.length() - 1);
                }
                groupsJson.append("]");
                jsonData.append("\"groups\":").append(groupsJson).append(",");
                String countQuery = "SELECT COUNT(*) FROM act WHERE type = 'Group' AND isDeleted = 'NO'";
                if(searchQuery != null && !searchQuery.isEmpty()) {
                    countQuery += " AND name LIKE ?";
                }
                try (PreparedStatement countStmt = conn.prepareStatement(countQuery)) {
                    if (searchQuery != null && !searchQuery.isEmpty()) {
                        countStmt.setString(1, "%" + searchQuery + "%");
                    }

                    try (ResultSet countRs = countStmt.executeQuery()) {
                        if (countRs.next()) {
                            int totalCount = countRs.getInt(1);
                            jsonData.append("\"totalCount\":").append(totalCount);
                        }
                    }
                }
            }
        } catch (SQLException e) {
            e.printStackTrace();
            return "{\"error\":\"Database error\"}";
        }
        jsonData.append("}");
        return jsonData.toString();
    }
}


