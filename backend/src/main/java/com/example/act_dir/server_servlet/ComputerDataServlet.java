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

public class ComputerDataServlet extends HttpServlet {
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
//            response.getWriter().write("Invalid JSON format");
            return;
        }

        String type = data.getString("type", null);
        if(type == null || !data.containsKey("computers")) {
            response.setStatus(HttpServletResponse.SC_BAD_REQUEST);
//            response.getWriter().write("Invalid input: 'type' or 'computers' key is missing");
            return;
        }

        JsonArray computers = data.getJsonArray("computers");

        List<String> successfulInserts = new ArrayList<>();
        List<String> failedInserts = new ArrayList<>();

        try(Connection conn = DBConnection.getConnection()) {
            for(JsonValue computerValue : computers) {
                JsonObject computer = computerValue.asJsonObject();
                String computerName = computer.getString("computerName", null);

                if (computerName == null) {
                    failedInserts.add(computerName);
                    continue;
                }

                if (computerExists(conn, computerName)) {
                    failedInserts.add(computerName);
                } else {
                    if (insertComputer(conn, type, computerName)) {
                        successfulInserts.add(computerName);
                    } else {
                        failedInserts.add(computerName);
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

    private boolean computerExists(Connection conn, String computerName) throws SQLException {
        String checkSql = "SELECT COUNT(*) FROM act WHERE name = ?";
        try (PreparedStatement checkStmt = conn.prepareStatement(checkSql)) {
            checkStmt.setString(1, computerName);

            try (ResultSet rs = checkStmt.executeQuery()) {
                if (rs.next() && rs.getInt(1) > 0) {
                    return true;
                }
            }
        }
        return false;
    }

    private boolean insertComputer(Connection conn, String type, String computerName) throws SQLException {
        String insertSql = "INSERT INTO act (type, name, isDeleted) VALUES (?, ?, 'NO')";
        try (PreparedStatement insertStmt = conn.prepareStatement(insertSql)) {
            insertStmt.setString(1, type);
            insertStmt.setString(2, computerName);

            int rowsInserted = insertStmt.executeUpdate();
            return rowsInserted > 0;
        }
    }
}