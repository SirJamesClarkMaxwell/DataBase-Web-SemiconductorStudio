from flask import Flask, request, send_file, jsonify, send_from_directory
from flask_cors import CORS
import json
import os
import shutil
from dbmanager import Manager





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
    

@app.route('/API/save', methods=['POST'])
def saveCred():
    data = request.json

    username = data.get('username')
    password = data.get('password')

    mng.get_conn()
    pass


@app.route('/API/createMesurement', methods=['POST'])
def insertMes():
    data = request.json

    storage_name = data.get('storage_name')
    temperature = data.get("temperature")
    characteristic = data.get("characteristic")
    try:
        mng.get_conn()

        mng.initialize_mesurement(storage_name, temperature, characteristic)

        return jsonify({"message": "Properly created mesurement table"}), 201

    except Exception as e:
        return jsonify({"error": "Failed to get data"}), 500
    


@app.route('/API/updateRec', methods = ['POST'])
def updRec():
    pass




if __name__ == "__main__":
    app.run(debug=True, port=5000)