<%@ page contentType="text/html; charset=UTF-8" pageEncoding="UTF-8" %>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Active Directory</title>
    <style>
        body {
            font-family: 'Arial', sans-serif;
            background-color: #f4f7fb;
            margin: 0;
            padding: 0;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            text-align: center;
        }

        .container {
            background-color: #fff;
            padding: 30px;
            border-radius: 8px;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
            text-align: center;
            width: 80%;
            max-width: 500px;
        }

        h1 {
            font-size: 2rem;
            color: #333;
            margin-bottom: 20px;
        }

        .link-button {
            display: inline-block;
            background-color: #007BFF;
            color: white;
            font-size: 1.2rem;
            padding: 15px 30px;
            border-radius: 5px;
            text-decoration: none;
            transition: background-color 0.3s ease;
        }

        .link-button:hover {
            background-color: #0056b3;
        }

        @media screen and (max-width: 768px) {
            h1 {
                font-size: 1.5rem;
            }

            .link-button {
                font-size: 1rem;
                padding: 12px 25px;
            }
        }
    </style>
</head>
<body>
<div class="container">
    <h1><%= "Welcome to Active Directory" %></h1>
    <a href="http://localhost:4200/home" class="link-button">Click Here</a>
</div>
</body>
</html>
