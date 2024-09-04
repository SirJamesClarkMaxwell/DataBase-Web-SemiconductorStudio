from flask import Flask, request, send_file, jsonify, send_from_directory
from flask_cors import CORS
import json
import os
import shutil
from dbmanager import Manager
import traceback




# login (GET)

# save (POST)

# downloadRec (GET)

# deleteRec (DELETE)

#postRec (POST)

#updateRec (PUT)

#collectThumb (GET)

app = Flask(__name__)
mng = Manager()



CORS(app, resources={
    "/*": {"origins": ["*"]}
    })

@app.route('/API/register', methods=['POST'])
def registerUser():

    try:
        data = request.json
        username = data.get('username')
        password = data.get('password')
        
        mng.get_conn()

        if mng.checkUser(username)==True:
            mng.registerNewUser(username, password)
            mng.close_conn()
            return jsonify({"message": "User registered successfully"}), 201
        else:
            return jsonify({"message": "user with this username already exists"}), 403
    except Exception as e:
        print(f"Error: {e}")
        return jsonify({"error": "Failed to register user"}), 500




@app.route('/API/login', methods=['GET'])
def loginUser():
    '''


    This method should also return token for further user authorizations

    
    '''
    try:
        data = request.json
        username = data.get('username')
        password = data.get('password')

        mng.get_conn()
        
        if not mng.checkUserPassword(username, password):
            token = mng.generate_token()
            return jsonify({"message": "User logged in successfully", "token": token}), 201
        else:
            return jsonify({"message": "there is no such user"}), 403
    except Exception as e:
        print(f"Error: {e}")
        return jsonify({"error": "Failed to login user"}), 500

@app.route('/API/getTable', methods = ['GET'])
def insertMesurement():
    try:
        data = request.json

        tablename = data.get('tablename')
        column_name = data.get('column_name')
        identifier = data.get('identifier')
        columns2get = data.get('columns2get')
        mng.get_conn()

        results = mng.get_from_db(tablename, column_name, identifier, columns2get)
        return jsonify({"message": "Got requested data from database", "data": results}), 201
    
    except Exception as e:
        return jsonify({"error": "Failed to get data"}), 500
    

@app.route('/API/createStorage', methods=['POST'])
def insertMes():
    data = request.json

    storage_name = data.get("storage_name")
    record_name = data.get("record_name")
    short_desc = data.get("short_desc")
    try:
        mng.get_conn()
        if mng.crate_storage(storage_name, record_name, short_desc):
            message = "Properly created storage space in main table"
            mng.close_conn()
            return jsonify({"message": message}), 201
        else:
            mng.close_conn()
            return jsonify({"message": "Given storage name or record name already exists"}), 500

    except Exception as e:
        return jsonify({"error": "Failed to get data"}), 500
    
@app.route('/API/insertData', methods=['POST'])
def insertData():
    data = request.json

    record_name = data.get('record_name')
    data_ = data.get('data')
    measurement = data.get('mes')  # IV or CV 

    try:
        mng.get_conn()

        if mng.conn is None:
            raise Exception("Failed to establish database connection")

        result = mng.insert_measurement(record_name, measurement, data_)

        if result is None:
            raise Exception("Failed to insert measurement")

        mng.close_conn()

        return jsonify({"message": "Data inserted successfully"}), 201

    except Exception as e:
        mng.close_conn()  # Ensure connection is closed even if an error occurs
        return jsonify({"error": str(e)}), 500

def make_json_serializable(data):
    if isinstance(data, list):
        return [make_json_serializable(item) for item in data]
    elif isinstance(data, dict):
        return {key: make_json_serializable(value) for key, value in data.items()}
    elif isinstance(data, set):
        return list(data)  # Convert set to list
    else:
        return data  # Return data as-is for other types
    

@app.route('/API/getConns', methods = ['GET'])
def getConns():
    try:
        mng.get_conn()
        # networking = mng.get_network()
        data = mng.get_network()
        serializable_data = make_json_serializable(data)

        mng.close_conn()

        return jsonify(serializable_data), 200


       # return jsonify({"message": "Connections received properly", "data": networking}), 201
    
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()  # Print full stack trace
        return {"error": "Failed to get data"}, 500



if __name__ == "__main__":
    app.run(debug=True, port=5000)