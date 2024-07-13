import binascii
import psycopg2
import os
from dotenv import load_dotenv
import hashlib
import base64
import uuid
import bcrypt
import uuid

load_dotenv()


class Manager:
    def __init__(self) -> None:
        self.host = os.getenv('HOST')
        self.db_name = os.getenv('DB_NAME')
        self.port = os.getenv('PORT')
        self.login = os.getenv('USER')
        self.options = "-c search_path=test"
        self.password = os.getenv('PASSWORD')
        print(f"Host: {self.host}")
        print(f"Database: {self.db_name}")
        print(f"Port: {self.port}")
        print(f"Login: {self.login}")
        print(f"Password: {self.password}")
        self.conn = None
        self.cursor=None


    def get_conn(self)->None:
        '''
        getting connection with database server to a specific schema (test)
        
        '''
        try:
            self.conn = psycopg2.connect(database=self.db_name,
                                    host=self.host,
                                    user=self.login,
                                    password = self.password,
                                    options=self.options,
                                    port=self.port)
            
            self.conn.set_client_encoding('UTF8')
            print("Connection successful")

            self.cursor = self.conn.cursor()

            self.cursor.execute("SELECT * FROM users;")

        except Exception as error:
            print(f"Error: {error}")
            
    def checkUser(self, username) -> bool:
        '''
        Checks if user is in database
        '''
        try:
            self.cursor.execute(f"SELECT user_name FROM users WHERE user_name='{username}';")

            result = self.cursor.fetchone()
        
            if result:
                print(f"User '{username}' exists.")
                return False
            else:
                print(f"User '{username}' does not exist.")
                return True

        except Exception as error:
            print(f"Error: {error}")
            return False
        

    def checkUserPassword(self, username, password) -> bool:
        '''
        Checks if user with given username exists in database
        Checks if password match the hash in database
        If user is authorized succesfully method returns True
        In any other case returns False
        '''
        try:
            self.cursor.execute(f"SELECT user_name FROM users WHERE user_name='{username}';")

            result = self.cursor.fetchone()
        
            if result:
                print(f"User '{username}' exists.")
                self.cursor.execute(f"SELECT user_password FROM users WHERE user_name='{username}'")
                hashed_pwd = self.cursor.fetchone()

                if self.check_password(password, hashed_pwd):
                    return False
                else:
                    print("passwords don't match")
                    return True
            else:
                print(f"User '{username}' does not exist.")
                self.close_conn()
                return True

        except Exception as error:
            self.close_conn()
            print(f"Error: {error}")
            return False
        

    def registerNewUser(self, username, password):
        '''
        Creating space for a new user
        user is added to users table and basic connections are made

        returns nothing 
        '''
        user_id = str(uuid.uuid4())
        storage_extension_id = str(uuid.uuid4())
        hashed_password = self.get_hashed_password(password)
        print(user_id)
        print(storage_extension_id)
        print(hashed_password)
        storage_id = str(uuid.uuid4())
        print(storage_id)
        try:
            
            self.cursor.execute(f"INSERT INTO test.users (user_id, user_name, user_password) VALUES(%s, %s, %s);", (user_id, username, hashed_password))
            self.cursor.execute(f"INSERT INTO test.storage_ (storage_id, storage_name) VALUES(%s, %s);", (storage_id, 'default_storage'))
            self.cursor.execute(f"INSERT INTO test.storage_extension (storage_extension_id, user_id_extension, storage_id) VALUES(%s, %s, %s);", (storage_extension_id, user_id, storage_id))
            self.cursor.execute(f"UPDATE test.users SET storage_extension_id = %s WHERE user_id = %s;", (storage_extension_id, user_id))
            # Commit the transaction
            self.conn.commit()

        except Exception as error:
            print(f"Error: {error}")

        
        

    def generate_token(self)->str:
        '''
        Generate unique token for further identifications
        '''
        
        return binascii.hexlify(os.urandom(20)).decode()

    def generate_uuid(self)->str:
        return str(uuid.uuid4())

    def get_hashed_password(self, plain_text_password):
    # Hash a password for the first time
    #   (Using bcrypt, the salt is saved into the hash itself)
        return bcrypt.hashpw(plain_text_password, bcrypt.gensalt())

    def check_password(self, plain_text_password, hashed_password):
    # Check hashed password. Using bcrypt, the salt is saved into the hash itself
        
        return bcrypt.checkpw(plain_text_password, hashed_password[0])
    



    def get_from_db(self, tablename, column_name, identifier, *args) -> str:
        '''
        Returns data from columns passed in "*args"
        while condition "WHERE column_name=identifier"
        
        '''
        if not args or not all(isinstance(arg, str) for arg in args):
            raise ValueError("You must provide column names as non-empty strings.")
        

        columns = ", ".join(args)
    
        if not isinstance(tablename, str) or not tablename.isidentifier():
            raise ValueError("Invalid table name.")

        query = f"SELECT {columns} FROM {tablename} WHERE {column_name} ='{identifier}';"
        
        try:

            self.cursor.execute(query)
            results = self.cursor.fetchall()
            
            return results
        
        except Exception as error:
            print(f"Error: {error}")
            return None

    def initialize_mesurement(self,  storage_name, temperature, characteristic)->None:
        '''
        z  tablicy storage_ dodajemy id do mesurement oraz mesurement_attribute i extension
        rozróżnienie trybu view i ten drugi
        '''

        attribute1 = self.generate_uuid()

        mes_tablename = self.generate_uuid()
        

        storage_id = self.get_from_db("storage_", "storage_name", storage_name, "storage_id")
        query_attribute1 = f"INSERT INTO  measurement_attribute (value, measurement_attribute_id, temperature, measurement_table_name, stoarge_id) VALUES ('{characteristic}', '{attribute1}', {temperature}, '{mes_tablename}', '{storage_id}');"
        mes_query = f"""
            CREATE TABLE {mes_tablename} (
                measurement_id SERIAL PRIMARY KEY, 
                measurement FLOAT, 
                next_measurement_id uuid 
            );
            """
        
        try:
            self.cursor.execute(query_attribute1)
            self.cursor.execute(mes_query)
            self.conn.commit()
            

        except Exception as e:
            print(f"Error: {e}")
            return None
    def change_storage_name(self, storage_name, previous_storage_name)->str:
        change_name_query = f"UPDATE test.storage_ SET storage_name = '{storage_name}' WHERE storage_name = '{previous_storage_name}';"
        try:
            self.cursor.execute(change_name_query)
            self.conn.commit()

            return "succesfulyl changed storage name"
            

        except Exception as e:
            print(f"Error: {e}")
            return None
        
    def insert_mesurements(self)->None:
        pass
    
    def close_conn(self)->None:
        '''
        closing connection to database

        to avoid server overloading should be executed after finishing every action
        '''
        if self.conn:
            self.cursor.close()
            self.conn.close()


#mng = Manager()

#mng.get_conn()