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

public class ComputerDataServlet extends HttpServlet {
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
            return;
        }

        String type = data.getString("type", null);
        if (type == null || !data.containsKey("computers")) {
            response.setStatus(HttpServletResponse.SC_BAD_REQUEST);
            return;
        }

        JsonArray computers = data.getJsonArray("computers");

        List<String> successfulInserts = new ArrayList<>();
        List<String> failedInserts = new ArrayList<>();

        try (Connection conn = DBConnection.getConnection()) {
            if (conn == null) {
                response.setStatus(HttpServletResponse.SC_INTERNAL_SERVER_ERROR);
                response.getWriter().write("Database connection failed.");
                return;
            }

            for (JsonValue computerValue : computers) {
                JsonObject computer = computerValue.asJsonObject();
                String computerName = computer.getString("computerName", null);
                String whenCreated = computer.getString("whenCreated", null);

                if (computerName == null || whenCreated == null) {
                    failedInserts.add(computerName);
                    continue;
                }

                if (computerExists(conn, computerName)) {
                    failedInserts.add(computerName);
                } else {
                    if (insertComputer(conn, type, computerName, whenCreated)) {
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

    private boolean insertComputer(Connection conn, String type, String computerName, String whenCreated) throws SQLException {
        // Convert whenCreated to the MySQL datetime format
        whenCreated = convertToMySQLDateFormat(whenCreated);

        String insertSql = "INSERT INTO act (type, name, whenCreated, isDeleted) VALUES (?, ?, ?, 'NO')";
        try (PreparedStatement insertStmt = conn.prepareStatement(insertSql)) {
            insertStmt.setString(1, type);
            insertStmt.setString(2, computerName);
            insertStmt.setString(3, whenCreated);

            int rowsInserted = insertStmt.executeUpdate();
            if (rowsInserted > 0) {
                System.out.println("Inserted computer: " + computerName);
                return true;
            } else {
                System.out.println("Failed to insert computer: " + computerName);
                return false;
            }
        }
    }

    // Method to convert whenCreated format to MySQL datetime format (yyyy-MM-dd HH:mm:ss)
    private String convertToMySQLDateFormat(String whenCreated) {
        SimpleDateFormat dateFormat = new SimpleDateFormat("dd-MM-yyyy HH:mm:ss");
        SimpleDateFormat mysqlDateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        try {
            Date parsedDate = dateFormat.parse(whenCreated);
            return mysqlDateFormat.format(parsedDate);
        } catch (ParseException e) {
            e.printStackTrace();
            return null;
        }
    }
}
