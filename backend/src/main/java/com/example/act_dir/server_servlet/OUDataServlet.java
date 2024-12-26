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
import java.util.ArrayList;
import java.util.List;
import com.example.act_dir.db.DBConnection;

public class OUDataServlet extends HttpServlet {
    @Override
    protected void doPost(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        StringBuilder jsonData = new StringBuilder();
        String line;
        try (BufferedReader reader = request.getReader()) {
            while ((line = reader.readLine()) != null) {
                jsonData.append(line);
            }
        }

        JsonObject data;
        try (JsonReader jsonReader = Json.createReader(new StringReader(jsonData.toString()))) {
            data = jsonReader.readObject();
        } catch (Exception e) {
            response.setStatus(HttpServletResponse.SC_BAD_REQUEST);
            response.getWriter().write("Invalid JSON format");
            return;
        }

        String type = data.getString("type", null);
        if (type == null || !data.containsKey("ous")) {
            response.setStatus(HttpServletResponse.SC_BAD_REQUEST);
            response.getWriter().write("Invalid input: 'type' or 'ous' key is missing");
            return;
        }

        JsonArray ous = data.getJsonArray("ous");

        List<String> successfulInserts = new ArrayList<>();
        List<String> failedInserts = new ArrayList<>();

        try (Connection conn = DBConnection.getConnection()) {
            for (JsonValue ouValue : ous) {
                JsonObject ou = ouValue.asJsonObject();
                String ouName = ou.getString("ouName", null);

                if (ouName == null) {
                    failedInserts.add(ouName);
                    continue;
                }

                if (ouExists(conn, ouName)) {
                    failedInserts.add(ouName);
                } else {
                    if (insertOU(conn, type, ouName)) {
                        successfulInserts.add(ouName);
                    } else {
                        failedInserts.add(ouName);
                    }
                }
            }
            response.setStatus(HttpServletResponse.SC_OK);
            response.getWriter().write("Successful inserts: " + successfulInserts.toString() + "\nFailed inserts: " + failedInserts.toString());

        } catch (SQLException e) {
            response.setStatus(HttpServletResponse.SC_INTERNAL_SERVER_ERROR);
            e.printStackTrace();
        }
    }

    private boolean ouExists(Connection conn, String ouName) throws SQLException {
        String checkSql = "SELECT COUNT(*) FROM act WHERE name = ?";
        try (PreparedStatement checkStmt = conn.prepareStatement(checkSql)) {
            checkStmt.setString(1, ouName);

            try (ResultSet rs = checkStmt.executeQuery()) {
                if (rs.next() && rs.getInt(1) > 0) {
                    return true;
                }
            }
        }
        return false;
    }

    private boolean insertOU(Connection conn, String type, String ouName) throws SQLException {
        String insertSql = "INSERT INTO act (type, name, isDeleted) VALUES (?, ?, 'NO')";
        try (PreparedStatement insertStmt = conn.prepareStatement(insertSql)) {
            insertStmt.setString(1, type);
            insertStmt.setString(2, ouName);

            int rowsInserted = insertStmt.executeUpdate();
            return rowsInserted > 0;
        }
    }
}
