package com.example.act_dir.graph;

import jakarta.servlet.ServletException;
import jakarta.servlet.http.HttpServlet;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;
import java.io.IOException;
import java.io.PrintWriter;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import com.example.act_dir.db.DBConnection;

public class ComputerCreationReportServlet extends HttpServlet {

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

        response.setContentType("application/json");

        try (PrintWriter out = response.getWriter()) {
            String jsonData = getComputerCreationDataAsJson();
            out.write(jsonData);
        }
    }

    private String getComputerCreationDataAsJson() {
        StringBuilder jsonData = new StringBuilder("{ \"data\":{");
        String query = "SELECT DATE(whenCreated) as createdDate, COUNT(*) as count " +
                "FROM act WHERE type = 'computer' AND isDeleted = 'NO' " +
                "GROUP BY DATE(whenCreated) ORDER BY DATE(whenCreated)";

        try (Connection conn = DBConnection.getConnection();
             PreparedStatement pstmt = conn.prepareStatement(query);
             ResultSet rs = pstmt.executeQuery()) {
            while (rs.next()) {
                String date = rs.getString("createdDate");
                int count = rs.getInt("count");
                jsonData.append("\"").append(date).append("\":{ \"count\":").append(count).append("},");
            }
            if (jsonData.charAt(jsonData.length() - 1) == ',') {
                jsonData.setLength(jsonData.length() - 1);
            }
            jsonData.append("}}");
        } catch (SQLException e) {
            e.printStackTrace();
            return "{\"error\":\"Database error\"}";
        }
        return jsonData.toString();
    }
}