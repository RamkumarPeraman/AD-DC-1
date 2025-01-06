package com.example.act_dir.create_servlet;

import jakarta.servlet.ServletException;
import jakarta.servlet.http.HttpServlet;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;
import javax.json.Json;
import javax.json.JsonObject;
import javax.json.JsonReader;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;

public class CreateComputerServlet extends HttpServlet {

    @Override
    protected void doOptions(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        addCorsHeaders(response);
        response.setStatus(HttpServletResponse.SC_OK);
    }

    @Override
    protected void doPost(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        addCorsHeaders(response);

        try (JsonReader jsonReader = Json.createReader(request.getInputStream())) {
            JsonObject json = jsonReader.readObject();

            String name = json.getString("name");
            String description = json.getString("description");
            String location = json.getString("location");

            String resultMessage = createComputer(name, description, location);

            response.setContentType("application/json");
            PrintWriter out = response.getWriter();
            JsonObject jsonResponse = Json.createObjectBuilder()
                    .add("status", resultMessage.equals("Computer created successfully") ? "success" : "failure")
                    .add("message", resultMessage)
                    .build();
            out.print(jsonResponse.toString());
            out.flush();
        } catch (Exception e) {
            e.printStackTrace();
            response.setStatus(HttpServletResponse.SC_INTERNAL_SERVER_ERROR);
            PrintWriter out = response.getWriter();
            JsonObject jsonResponse = Json.createObjectBuilder()
                    .add("status", "failure")
                    .add("message", e.getMessage())
                    .build();
            out.print(jsonResponse.toString());
            out.flush();
        }
    }

    private void addCorsHeaders(HttpServletResponse response) {
        response.setHeader("Access-Control-Allow-Origin", "*");
        response.setHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        response.setHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
    }

    private String createComputer(String name, String description, String location) {
        try {
            ProcessBuilder processBuilder = new ProcessBuilder("/home/ram-pt7749/Music/prom/agent/create/computerCreate", name, description, location);
            processBuilder.redirectErrorStream(true);
            Process process = processBuilder.start();

            BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
            StringBuilder output = new StringBuilder();
            String line;
            while ((line = reader.readLine()) != null) {
                output.append(line);
            }
            int exitCode = process.waitFor();
            if (exitCode != 0) {
                System.err.println("Error creating computer. Exit code: " + exitCode);
                System.err.println(output.toString());
                return output.toString();
            }
            return output.toString().contains("Computer already exists") ? "Computer already exists" : "Computer created successfully";
        } catch (Exception e) {
            e.printStackTrace();
            return e.getMessage();
        }
    }
}