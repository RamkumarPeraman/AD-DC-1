package com.example.act_dir.cors_filter;
import jakarta.servlet.http.HttpServletResponse;

public class CORS_Filter {
        public static void setCORSHeaders(HttpServletResponse response) {
                response.setHeader("Access-Control-Allow-Origin", "*");
                response.setHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS, PUT, DELETE");
                response.setHeader("Access-Control-Allow-Headers", "Content-Type");
        }
}