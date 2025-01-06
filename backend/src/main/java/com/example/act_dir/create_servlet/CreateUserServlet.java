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
public class CreateUserServlet extends HttpServlet {

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

            String firstName = json.getString("firstName");
            String lastName = json.getString("lastName");
            String mail = json.getString("mail");
            String phnnumber = json.getString("phnnumber");
            String description = json.getString("description");
            String displayname = json.getString("displayname");

            System.out.println("Creating user with the following details:");
            System.out.println("First Name: " + firstName);
            System.out.println("Last Name: " + lastName);
            System.out.println("Mail: " + mail);
            System.out.println("Phone Number: " + phnnumber);
            System.out.println("Description: " + description);
            System.out.println("Display Name: " + displayname);

            String resultMessage = createUser(firstName, lastName, mail, phnnumber, description, displayname);

            System.out.println("Result Message: " + resultMessage);

            response.setContentType("application/json");
            PrintWriter out = response.getWriter();
            JsonObject jsonResponse = Json.createObjectBuilder()
                    .add("status", resultMessage.equals("User created successfully") ? "success" : "failure")
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
    private String createUser(String firstName, String lastName, String mail, String phnnumber, String description, String displayname) {
        try {
            ProcessBuilder processBuilder = new ProcessBuilder("/home/ram-pt7749/Music/prom/agent/create/userCreate", firstName, lastName, mail, phnnumber, description, displayname);
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
                String errorMessage = "Error creating user" + exitCode + "-Output: " + output.toString();
                System.err.println(errorMessage);
                return errorMessage;
            }
            return output.toString().contains("User already exists") ? "User already exists" : "User created successfully";
        } catch (Exception e) {
            e.printStackTrace();
            return e.getMessage();
        }
    }
}