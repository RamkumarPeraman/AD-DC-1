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

public class UserServlet extends HttpServlet {
    @Override
    protected void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        response.setHeader("Access-Control-Allow-Origin", "*");
        response.setHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        response.setHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
        if ("OPTIONS".equalsIgnoreCase(request.getMethod())) {
            response.setStatus(HttpServletResponse.SC_OK);
            return;
        }
        response.setContentType("application/json");

        String searchQuery = request.getParameter("search");
        String sortBy = request.getParameter("sortBy");
        if (sortBy == null) {
            sortBy = "name ASC";
        } else {
            switch (sortBy) {
                case "asc-desc":
                    sortBy = "name ASC";
                    break;
                case "desc-asc":
                    sortBy = "name DESC";
                    break;
                case "new-old":
                    sortBy = "whenCreated DESC";
                    break;
                case "old-new":
                    sortBy = "whenCreated ASC";
                    break;
            }
        }

        try (PrintWriter out = response.getWriter()) {
            String jsonData = getUserNamesAsJson(searchQuery, sortBy);
            out.write(jsonData);
        }
    }

    private String getUserNamesAsJson(String searchQuery, String sortBy) {
        StringBuilder jsonData = new StringBuilder("[");
        StringBuilder queryBuilder = new StringBuilder("SELECT name FROM act WHERE type = 'User' AND isDeleted = 'NO'");
        if (searchQuery != null && !searchQuery.isEmpty()) {
            queryBuilder.append(" AND name LIKE ?");
        }
        queryBuilder.append(" ORDER BY ").append(sortBy);

        try (Connection conn = DBConnection.getConnection();
             PreparedStatement pstmt = conn.prepareStatement(queryBuilder.toString())) {
            if (searchQuery != null && !searchQuery.isEmpty()) {
                pstmt.setString(1, "%" + searchQuery + "%");
            }
            ResultSet rs = pstmt.executeQuery();
            while (rs.next()) {
                jsonData.append("{\"name\":\"").append(rs.getString("name")).append("\"},");
            }
            if (jsonData.length() > 1) {
                jsonData.setLength(jsonData.length() - 1);
            }
            jsonData.append("]");
        } catch (SQLException e) {
            e.printStackTrace();
            return "{\"error\":\"Database error\"}";
        }
        return jsonData.toString();
    }
}
