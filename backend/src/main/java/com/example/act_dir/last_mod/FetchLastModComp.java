package com.example.act_dir.last_mod;

import jakarta.servlet.ServletException;
import jakarta.servlet.http.HttpServlet;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

public class FetchLastModComp extends HttpServlet {

    @Override
    protected void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        response.setHeader("Access-Control-Allow-Origin", "*");
        response.setHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        response.setHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
        if ("OPTIONS".equalsIgnoreCase(request.getMethod())) {
            response.setStatus(HttpServletResponse.SC_OK);
            return;
        }
        String userName = request.getParameter("objName");
        response.setContentType("application/json");
        String userDetails = fetchLastModDetails(userName);
        response.getWriter().write(userDetails);
    }
    private String fetchLastModDetails(String userName) {
        String result = "{}";
        try {
            ProcessBuilder processBuilder = new ProcessBuilder("/home/ram-pt7749/Music/prom/agent/lastMod/compLastMod", userName);
            processBuilder.redirectErrorStream(true);
            Process process = processBuilder.start();
            BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
            StringBuilder output = new StringBuilder();
            String line;
            while ((line = reader.readLine()) != null) {
                output.append(line);
            }
            int exitCode = process.waitFor();
            if (exitCode == 0) {
                result = output.toString();
            } else {
                System.err.println("Error fetching user details. Exit code: " + exitCode);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        System.out.println("User details----------: " + result);
        return result;
    }
}
