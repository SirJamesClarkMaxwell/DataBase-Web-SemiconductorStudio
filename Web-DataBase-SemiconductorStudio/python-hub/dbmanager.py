import binascii
import psycopg2
import os
from dotenv import load_dotenv
import hashlib
import base64
import uuid
import bcrypt
import uuid
from parser import Parser
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
    



    def get_from_db(self, tablename, column_name, identifier, all_, *args) -> str:
        '''
        Returns data from columns passed in "*args"
        while condition "WHERE column_name=identifier"
        
        '''
        if not args or not all(isinstance(arg, str) for arg in args):
            raise ValueError("You must provide column names as non-empty strings.")
        

        columns = ", ".join(args)
    
        if not isinstance(tablename, str) or not tablename.isidentifier():
            raise ValueError("Invalid table name.")
        if all_ == True:
            query = f"SELECT {columns} FROM {tablename};"
        else:
            query = f"SELECT {columns} FROM {tablename} WHERE {column_name} ='{identifier}';"
        
        try:

            self.cursor.execute(query)
            results = self.cursor.fetchall()
            self.conn.commit()
            return results
        
        except Exception as error:
            print(f"Error: {error}")
            return None

    def crate_storage(self, storage_name, record_name, short_desc) -> bool:
        uuid = self.generate_uuid()
        storage_name = storage_name.replace(" ", "_")
        
        # Check if the record_name and storage_name already exist in the main_table
        check_query = f"""
        SELECT COUNT(*) FROM test.main_table 
        WHERE record_name = '{record_name}';
        """
        
        try:
            # Execute the check query
            self.cursor.execute(check_query)
            result = self.cursor.fetchone()

            # If the combination already exists, return an error
            if result[0] > 0:
                print(f"Error: Record with record_name '{record_name}' and storage_name '{storage_name}' already exists.")
                return False

            # If the combination doesn't exist, proceed with the insertions
            query = f"""
            INSERT INTO test.main_table (id_, record_name, storages, short_description) 
            VALUES ('{uuid}', '{record_name}', '{storage_name}', '{short_desc}');
            """
            query2 = f"CREATE TABLE test.{storage_name} (id uuid PRIMARY KEY, name varchar(30));"
            query3 = f"INSERT INTO test.{storage_name} (id, name) VALUES ('{uuid}', '{record_name}');"
            
            self.cursor.execute(query)
            self.cursor.execute(query2)
            self.cursor.execute(query3)
            self.conn.commit()
            
            return True
        except Exception as error:
            print(f"Error: {error}")
            return False
        
    def insert_measurement(self, record_name, measurement_type, data):
        try:
            # Use parameterized queries
            query1 = "SELECT id_ FROM test.main_table WHERE record_name = %s"
            self.cursor.execute(query1, (record_name,))
            result1 = self.cursor.fetchone()
            
            if result1 is None:
                raise ValueError(f"No record found with name: {record_name}")
            
            id_ = str(result1[0])
            print(result1, flush=True)
            
            # Check if the record already exists in the measurement table
            query2 = f"""
                SELECT id, mes_num, next_mesurement 
                FROM test.{measurement_type} 
                WHERE name = %s 
                ORDER BY mes_num DESC 
                LIMIT 1
            """
            self.cursor.execute(query2, (record_name,))
            existing_record = self.cursor.fetchone()
            
            new_id = str(uuid.uuid4())  # Generate a new UUID for the new measurement
            
            if existing_record:
                # If record exists, use the next mes_num
                new_mes_num = existing_record[1] + 1
                
                # Insert new measurement
                query3 = f"INSERT INTO test.{measurement_type} (id, name, data, mes_num) VALUES (%s, %s, %s, %s)"
                self.cursor.execute(query3, (new_id, record_name, data, new_mes_num))
                
                # Update the previous record's next_mesurement
                query4 = f"UPDATE test.{measurement_type} SET next_mesurement = %s WHERE id = %s"
                self.cursor.execute(query4, (new_id, existing_record[0]))
            else:
                # If no record exists, insert as a new record with mes_num 1
                query5 = f"INSERT INTO test.{measurement_type} (id, name, data, mes_num) VALUES (%s, %s, %s, %s)"
                self.cursor.execute(query5, (new_id, record_name, data, 1))
            
                # Update the main table
                query6 = f"UPDATE test.main_table SET {measurement_type} = %s WHERE id_ = %s"
                self.cursor.execute(query6, (new_id, id_))
            
            self.conn.commit()
            return True
                

        
        except Exception as error:
            print(f"Error in insert_measurement: {error}")
            self.conn.rollback()
            raise

    def get_network(self):
        main_table = self.get_from_db("main_table", "a", "a", True, "storages", "record_name", "iv", "cv")
        curr = self.get_from_db("iv", "", "", True, "name", "mes_num")
        c = self.get_from_db("cv", "", "", True, "name", "mes_num")
        storages_data = []
        result = []

        try:
            for i in range(len(main_table)):
                storage = main_table[i][0]
                record = main_table[i][1]
                query = f"SELECT * FROM test.{storage}"
                self.cursor.execute(query)
                storages = self.cursor.fetchall()
                storages_data.append({"storage name": storage, "record name": storages[0][1]})

                mes_data = []  # Reset mes_data for each record

                # Handling I(V) characteristics
                iv_query = f"""
                SELECT * FROM test.iv WHERE name = '{record}';
                """
                self.cursor.execute(iv_query)
                iv_records = self.cursor.fetchall()

                if iv_records:
                    iv_details = []
                    for iv_record in iv_records:
                        iv_values = iv_record[2]  # Assuming the third column contains the list of I(V) values
                        iv_details.append({
                            "IV record ID": iv_record[0],  # UUID of the I(V) record
                            "IV values": iv_values
                        })
                    mes_data.append({
                        "I(V) characteristics": iv_details
                    })
                else:
                    mes_data.append({f"No I(V) characteristics for given storage: {storage}"})

                # Handling C(V) characteristics
                cv_query = f"""
                SELECT * FROM test.cv WHERE name = '{record}';
                """
                self.cursor.execute(cv_query)
                cv_records = self.cursor.fetchall()

                if cv_records:
                    cv_details = []
                    for cv_record in cv_records:
                        cv_values = cv_record[2]  # Assuming the third column contains the list of C(V) values
                        cv_details.append({
                            "CV record ID": cv_record[0],  # UUID of the C(V) record
                            "CV values": cv_values
                        })
                    mes_data.append({
                        "C(V) characteristics": cv_details
                    })
                else:
                    mes_data.append({f"No C(V) characteristics for given storage: {storage}"})

                result.append({
                    "storage_name": storage,
                    "record_name": record,
                    f"Data for storage: {storage}": mes_data
                })

            result.append({"storages_data": storages_data})
            print(result)
            return result
        except Exception as error:
            print(f"Error: {error}")
            return None
    def close_conn(self)->None:
        '''
        closing connection to database

        to avoid server overloading should be executed after finishing every action
        '''
        if self.conn:
            self.cursor.close()
            self.conn.close()


mng = Manager()

mng.get_conn()

#mng.crate_storage("test_storage", "record1", "Jest to testowy opis pierwszego storage z testowym recordem")

#mng.insert_mesurement("record1", "cv", "[1,2,3,4,5,6,7,8,9,10]")

mng.get_network()