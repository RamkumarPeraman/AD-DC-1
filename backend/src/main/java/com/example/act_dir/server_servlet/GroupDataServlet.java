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
import java.sql.*;
import java.text.SimpleDateFormat;

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
        System.out.print(jsonData);
        JsonObject data;
        try (JsonReader jsonReader = Json.createReader(new StringReader(jsonData.toString()))) {
            data = jsonReader.readObject();
        } catch(Exception e) {
            response.setStatus(HttpServletResponse.SC_BAD_REQUEST);
            return;
        }

        String type = data.getString("type", null);
        String groupName = data.getString("groupName", null);
        String whenCreatedStr = data.getString("whenCreated", null);

        if(type == null || groupName == null || whenCreatedStr == null) {
            response.setStatus(HttpServletResponse.SC_BAD_REQUEST);
            return;
        }

        // Convert whenCreated to a Timestamp
        Timestamp whenCreated = null;
        try {
            SimpleDateFormat sdf = new SimpleDateFormat("dd-MM-yyyy HH:mm:ss");
            java.util.Date date = sdf.parse(whenCreatedStr);
            whenCreated = new Timestamp(date.getTime());
        } catch (Exception e) {
            response.setStatus(HttpServletResponse.SC_BAD_REQUEST);
            response.getWriter().write("Invalid 'whenCreated' format. Expected format: dd-MM-yyyy HH:mm:ss");
            return;
        }

        try(Connection conn = DBConnection.getConnection()) {
            if (groupExists(conn, groupName)) {
                response.setStatus(HttpServletResponse.SC_CONFLICT);
                response.getWriter().write("Group already exists: " + groupName + "\n");
            } else {
                if (insertGroup(conn, type, groupName, whenCreated)) {
                    response.setStatus(HttpServletResponse.SC_OK);
                    response.getWriter().write("Group successfully inserted: " + groupName +"\n");
                } else {
                    response.setStatus(HttpServletResponse.SC_INTERNAL_SERVER_ERROR);
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

    private boolean insertGroup(Connection conn, String type, String groupName, Timestamp whenCreated) throws SQLException {
        String insertSql = "INSERT INTO act (type, name, whenCreated, isDeleted) VALUES (?, ?, ?, 'NO')";
        try (PreparedStatement insertStmt = conn.prepareStatement(insertSql)) {
            insertStmt.setString(1, type);
            insertStmt.setString(2, groupName);
            insertStmt.setTimestamp(3, whenCreated);

            int rowsInserted = insertStmt.executeUpdate();
            return rowsInserted > 0;
        }
    }
}
