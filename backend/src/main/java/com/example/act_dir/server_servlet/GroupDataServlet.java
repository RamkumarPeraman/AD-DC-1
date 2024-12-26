package com.example.act_dir.server_servlet;

import jakarta.servlet.ServletException;
import jakarta.servlet.http.HttpServlet;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;
import javax.json.*;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.StringReader;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import com.example.act_dir.db.DBConnection;

public class GroupDataServlet extends HttpServlet {
    @Override
    protected void doPost(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        StringBuilder jsonData = new StringBuilder();
        String line;
        try (BufferedReader reader = request.getReader()) {
            while((line = reader.readLine()) != null) {
                jsonData.append(line);
            }
        }
        JsonObject data;
        try (JsonReader jsonReader = Json.createReader(new StringReader(jsonData.toString()))) {
            data = jsonReader.readObject();
        } catch(Exception e) {
            response.setStatus(HttpServletResponse.SC_BAD_REQUEST);
            response.getWriter().write("Invalid JSON format");
            return;
        }

        String type = data.getString("type", null);
        String groupName = data.getString("groupName", null);

        if(type == null || groupName == null) {
            response.setStatus(HttpServletResponse.SC_BAD_REQUEST);
            response.getWriter().write("Invalid input: 'type' or 'groupName' key is missing");
            return;
        }

        try(Connection conn = DBConnection.getConnection()) {
            if (groupExists(conn, groupName)) {
                response.setStatus(HttpServletResponse.SC_CONFLICT);
                response.getWriter().write("Group already exists: " + groupName + "\n");
            } else {
                if (insertGroup(conn, type, groupName)) {
                    response.setStatus(HttpServletResponse.SC_OK);
                    response.getWriter().write("Group successfully inserted: " + groupName +"\n");
                } else {
                    response.setStatus(HttpServletResponse.SC_INTERNAL_SERVER_ERROR);
//                    response.getWriter().write("Failed to insert group: " + groupName);
                }
            }
        } catch (SQLException e) {
            response.setStatus(HttpServletResponse.SC_INTERNAL_SERVER_ERROR);
            e.printStackTrace();
        }
    }

    private boolean groupExists(Connection conn, String groupName) throws SQLException {
        String checkSql = "SELECT COUNT(*) FROM act WHERE name = ?";
        try (PreparedStatement checkStmt = conn.prepareStatement(checkSql)) {
            checkStmt.setString(1, groupName);

            try (ResultSet rs = checkStmt.executeQuery()) {
                if (rs.next() && rs.getInt(1) > 0) {
                    return true;
                }
            }
        }
        return false;
    }

    private boolean insertGroup(Connection conn, String type, String groupName) throws SQLException {
        String insertSql = "INSERT INTO act (type, name, isDeleted) VALUES (?, ?, 'NO')";
        try (PreparedStatement insertStmt = conn.prepareStatement(insertSql)) {
            insertStmt.setString(1, type);
            insertStmt.setString(2, groupName);

            int rowsInserted = insertStmt.executeUpdate();
            return rowsInserted > 0;
        }
    }
}
