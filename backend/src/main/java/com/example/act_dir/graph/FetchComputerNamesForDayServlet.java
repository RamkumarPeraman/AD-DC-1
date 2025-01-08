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

public class FetchComputerNamesForDayServlet extends HttpServlet {

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

        String day = request.getParameter("day");
        if (day == null) {
            response.sendError(HttpServletResponse.SC_BAD_REQUEST, "Day parameter is missing");
            return;
        }

        try (PrintWriter out = response.getWriter()) {
            String jsonData = getComputerNamesForDayAsJson(day);
            out.write(jsonData);
        }
    }

    private String getComputerNamesForDayAsJson(String day) {
        StringBuilder jsonData = new StringBuilder("{ \"computers\":[");
        String query = "SELECT name, whenCreated FROM act WHERE type = 'computer' AND isDeleted = 'NO' AND DATE(whenCreated) = ?";

        try (Connection conn = DBConnection.getConnection();
             PreparedStatement pstmt = conn.prepareStatement(query)) {
            pstmt.setString(1, day);
            try (ResultSet rs = pstmt.executeQuery()) {
                while (rs.next()){
                    String name = rs.getString("name");
                    String whenCreated = rs.getString("whenCreated");
                    jsonData.append("{\"name\":\"").append(name).append("\",\"whenCreated\":\"").append(whenCreated).append("\"},");
                }
                if (jsonData.charAt(jsonData.length() - 1) == ',') {
                    jsonData.setLength(jsonData.length() - 1);
                }
                jsonData.append("]}");
            }
        } catch (SQLException e) {
            e.printStackTrace();
            return "{\"error\":\"Database error\"}";
        }
        return jsonData.toString();
    }
}