    package com.example.act_dir.server_servlet;

import jakarta.servlet.ServletException;
import jakarta.servlet.http.HttpServlet;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;

import javax.json.Json;
import javax.json.JsonArray;
import javax.json.JsonObject;
import javax.json.JsonReader;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.StringReader;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.List;

import com.example.act_dir.db.DBConnection;

public class DeletedObjDataServlet extends HttpServlet {

    @Override
    protected void doPost(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        // Read the input string from the request
        StringBuilder jsonBuffer = new StringBuilder();
        String line;
        try (BufferedReader reader = request.getReader()) {
            while ((line = reader.readLine()) != null) {
                jsonBuffer.append(line);
            }
        }

        String jsonString = jsonBuffer.toString();

        // Parse the JSON string using javax.json
        JsonObject data;
        try (JsonReader jsonReader = Json.createReader(new StringReader(jsonString))) {
            data = jsonReader.readObject();
        } catch (Exception e) {
            response.setStatus(HttpServletResponse.SC_BAD_REQUEST);
            response.getWriter().write("Invalid JSON format");
            return;
        }

        // Check for the presence of the 'deletedObjects' key
        if (!data.containsKey("deletedObjects")) {
            response.setStatus(HttpServletResponse.SC_BAD_REQUEST);
            response.getWriter().write("'deletedObjects' key is missing");
            return;
        }

        JsonArray deletedObjects = data.getJsonArray("deletedObjects");
        List<String> successfulInserts = new ArrayList<>();
        List<String> failedInserts = new ArrayList<>();

        try (Connection conn = DBConnection.getConnection()) {
            conn.setAutoCommit(false);

            for (int i = 0; i < deletedObjects.size(); i++) {
                JsonObject deletedObject = deletedObjects.getJsonObject(i);
                String type = deletedObject.getString("type", null);
                String objectName = null;
                String description = deletedObject.getString("description", null);

                if (type != null) {
                    switch (type.toLowerCase()) {
                        case "user":
                            objectName = deletedObject.getString("userName", null);
                            break;
                        case "group":
                            objectName = deletedObject.getString("groupName", null);
                            break;
                        case "computer":
                            objectName = deletedObject.getString("computerName", null);
                            break;
                        default:
                            continue;
                    }
                }

                if (objectName == null || description == null) {
                    failedInserts.add(objectName != null ? objectName : type);
                    continue;
                }

                // Check if the object already exists in the database
                if (objectExists(conn, objectName, description)) {
                    failedInserts.add(objectName);
                } else {
                    // Insert the object into the database
                    if (insertDeletedObject(conn, type, objectName, description)) {
                        successfulInserts.add(objectName);
                    } else {
                        failedInserts.add(objectName);
                    }
                }
            }

            conn.commit();
            response.setStatus(HttpServletResponse.SC_OK);
            response.getWriter().write("Successful inserts: " + successfulInserts.toString() + "\nFailed inserts: " + failedInserts.toString());

        } catch (SQLException e) {
            response.setStatus(HttpServletResponse.SC_INTERNAL_SERVER_ERROR);
            e.printStackTrace();
        } finally {
            try {
                DBConnection.getConnection().setAutoCommit(true);
            } catch (SQLException e) {
                e.printStackTrace();
            }
        }
    }

    // Check if the object already exists in the database
    private boolean objectExists(Connection conn, String objectName, String description) throws SQLException {
        String checkSql = "SELECT COUNT(*) FROM act_dit WHERE name = ? AND description = ?";
        try (PreparedStatement checkStmt = conn.prepareStatement(checkSql)) {
            checkStmt.setString(1, objectName);
            checkStmt.setString(2, description);

            try (ResultSet rs = checkStmt.executeQuery()) {
                return rs.next() && rs.getInt(1) > 0;
            }
        }
    }

    // Insert the deleted object into the database
    private boolean insertDeletedObject(Connection conn, String type, String objectName, String description) throws SQLException {
        String insertSql = "INSERT INTO act_dit (type, name, description, isDeleted) VALUES (?, ?, ?, 'YES')";
        try (PreparedStatement insertStmt = conn.prepareStatement(insertSql)) {
            insertStmt.setString(1, type);
            insertStmt.setString(2, objectName);
            insertStmt.setString(3, description);

            int rowsInserted = insertStmt.executeUpdate();
            return rowsInserted > 0;
        }
    }
}
