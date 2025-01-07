package com.example.act_dir.server_servlet;

import jakarta.servlet.ServletException;
import jakarta.servlet.http.HttpServlet;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;
import javax.json.*;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.StringReader;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.ArrayList;
import java.util.List;

import com.example.act_dir.db.DBConnection;

public class UserDataServlet extends HttpServlet {

    @Override
    protected void doPost(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        StringBuilder jsonData = new StringBuilder();
        String line;

        // Reading JSON data from request
        try (BufferedReader reader = request.getReader()) {
            while ((line = reader.readLine()) != null) {
                jsonData.append(line);
            }
        }

        // Parsing JSON data
        JsonObject data;
        try (JsonReader jsonReader = Json.createReader(new StringReader(jsonData.toString()))) {
            data = jsonReader.readObject();
        } catch (Exception e) {
            response.setStatus(HttpServletResponse.SC_BAD_REQUEST);
            response.setContentType("application/json");
            return;
        }

        // Validating required fields in JSON data
        String type = data.getString("type", null);
        if (type == null || !data.containsKey("Users")) {
            response.setStatus(HttpServletResponse.SC_BAD_REQUEST);
            response.setContentType("application/json");
            return;
        }

        JsonArray users = data.getJsonArray("Users");

        List<String> successfulInserts = new ArrayList<>();
        List<String> failedInserts = new ArrayList<>();
        List<String> failedMessages = new ArrayList<>();

        // Database operations
        try (Connection conn = DBConnection.getConnection()) {
            for (JsonValue userValue : users) {
                JsonObject user = userValue.asJsonObject();
                String userName = user.getString("userName", null);
                String whenCreatedStr = user.getString("whenCreated", null);

                if (userName == null) {
                    failedInserts.add("null userName");
                    failedMessages.add("User name is missing");
                    continue;
                }
                if (whenCreatedStr == null) {
                    failedInserts.add("null whenCreated");
                    failedMessages.add("User name is missing");
                    continue;
                }

                if (userExists(conn, userName)) {
                    failedInserts.add(userName);
                    response.getWriter().write("User already exists: " + userName + "\n");
                } else {
                    if (insertUser(conn, type, userName, whenCreatedStr)) {
                        response.getWriter().write("User Successfully inserted: " + userName + "\n");
                    } else {
                        response.getWriter().write("Failed to insert User: " + userName + "\n");
                    }
                }
            }
            response.setStatus(HttpServletResponse.SC_OK);
            response.setContentType("application/json");

        } catch (SQLException e) {
            response.setStatus(HttpServletResponse.SC_INTERNAL_SERVER_ERROR);
            response.setContentType("application/json");
            response.getWriter().write("{\"error\":\"Database error occurred\"}");
            e.printStackTrace();
        }
    }

    private boolean userExists(Connection conn, String userName) throws SQLException {
        String checkSql = "SELECT COUNT(*) FROM act WHERE name = ?";
        try (PreparedStatement checkStmt = conn.prepareStatement(checkSql)) {
            checkStmt.setString(1, userName);

            try (ResultSet rs = checkStmt.executeQuery()) {
                if (rs.next() && rs.getInt(1) > 0) {
                    return true;
                }
            }
        }
        return false;
    }

    private boolean insertUser(Connection conn, String type, String userName, String whenCreated) throws SQLException {
        String whenCreatedFormatted = convertDateFormat(whenCreated);
        if (whenCreatedFormatted == null) {
            return false;  // If the date format conversion failed, do not insert
        }

        String insertSql = "INSERT INTO act (type, name, isDeleted, whenCreated) VALUES (?, ?, 'NO', ?)";
        try (PreparedStatement insertStmt = conn.prepareStatement(insertSql)) {
            insertStmt.setString(1, type);
            insertStmt.setString(2, userName);
            insertStmt.setString(3, whenCreatedFormatted);  // Use the formatted date

            int rowsInserted = insertStmt.executeUpdate();
            return rowsInserted > 0;
        }
    }
    private String convertDateFormat(String whenCreatedStr) {
        try {
            SimpleDateFormat inputFormat = new SimpleDateFormat("dd-MM-yyyy HH:mm:ss");
            Date date = inputFormat.parse(whenCreatedStr);
            SimpleDateFormat outputFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
            return outputFormat.format(date);
        } catch (ParseException e) {
            e.printStackTrace();
            return null;
        }
    }
}
