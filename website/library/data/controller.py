from flask import Blueprint
import os
import psycopg2
from flask import Flask, render_template, request, jsonify, Blueprint

db_name = "demo"
db_user = "postgres"
db_password = "zenon"
db_host = "localhost"
db_port = "5432"
tb_name = "infor"

def connectDB():
    conn = psycopg2.connect(
        dbname=db_name,
        user=db_user,
        password=db_password,
        host=db_host,
        port=db_port
    )
    return conn

data = Blueprint("data", __name__)

# @data.route("/get-data")
# def get_data()

@data.route("/get-data", methods=['GET'])
def insert_db():

    # Data to insert to database
    data_to_insert = (1,100 ,10, 10, 20, 0, "name_1" )
    try:
        # Connect to the PostgreSQL database
        connection = psycopg2.connect(**db_name)

        # Create a cursor object
        cursor = connection.cursor()

        # SQL statement to insert data into the table
        insert_query = "INSERT INTO " + tb_name + \
            " (id, p, u, i, so_dien, status, nodename) VALUES (%d, %f, %f, %f, %d, %d, %s)"

        cursor.execute(insert_query, data_to_insert)

    except (Exception, psycopg2.Error) as error:
        print(f"Error inserting data: {error}")

    