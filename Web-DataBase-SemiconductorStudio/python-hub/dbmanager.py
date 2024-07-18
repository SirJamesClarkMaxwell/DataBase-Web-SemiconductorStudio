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

    def crate_storage(self, storage_name, record_name, short_desc):
        uuid = self.generate_uuid()

        query = f"INSERT INTO test.main_table (id_, record_name, storages, short_description) VALUES ('{uuid}', '{record_name}', '{storage_name}', '{short_desc}');"
        query2 = f"CREATE TABLE test.{storage_name} (id uuid PRIMARY KEY, name varchar(30));"
        query3 = f"INSERT INTO test.{storage_name} (id, name) VALUES ('{uuid}', '{record_name}');"
        
        try:

            self.cursor.execute(query)
            self.cursor.execute(query2)
            self.cursor.execute(query3)
            self.conn.commit()
            
        
        except Exception as error:
            print(f"Error: {error}")
            return None
        
    def insert_mesurement(self, recordname, mes, data)->None:
        query1 = f"SELECT id_ FROM test.main_table WHERE record_name = '{recordname}'"
        try:

            self.cursor.execute(query1)
            result1 = self.cursor.fetchall()
            id_ = str(result1[0][0])

            query2 = f"INSERT INTO test.{mes} (id, name, data, mes_num) VALUES ('{id_}', '{recordname}', '{data}', 1)"
            query3 = f"UPDATE test.main_table SET {mes} = '{id_}' WHERE id_ = '{id_}';"
            self.cursor.execute(query2)
            self.cursor.execute(query3)
            self.conn.commit()
            return None
        
        except Exception as error:
            print(f"Error: {error}")
            return None
        
    def insert_another_mes(self, mes, data, uuid):

        amount = self.get_from_db(f"{mes}", "id", f"{uuid}", False, "mes_num")
        nex = amount + 1
        try:

            query1 = f"CREATE TABLE test.{nex} (id uuid PRIMARY KEY, data text, next_mes uuid, mes_num int);"
            self.cursor.execute(query1)

            query2 = f"ALTER TABLE text.{amount} ADD CONSTRAINT FOREIGN KEY fk_ivn FOREIGN KEY (next_mesurement) REFERENCES test.{nex}(id);"
            self.cursor.execute(query2)
            query3 = f"INSERT INTO TABLE test.{nex} (data) VALUES ('{data}');"
            query4 = f"UPDATE test.iv SET mes_num = '{nex}' WHERE id_ = '{uuid}';"
            self.cursor.execute(query3)
            self.cursor.execute(query4)

            results = self.cursor.fetchall()
            self.conn.commit()
            return results
        
        except Exception as error:
            print(f"Error: {error}")
            return None
    def get_network(self):
        main_table = self.get_from_db("main_table", "a", "a", True, "storages", "record_name", "iv", "cv")
        #print(main_table)
        curr = self.get_from_db("iv", "", "", True, "name", "mes_num")
        c = self.get_from_db("cv", "", "", True, "name", "mes_num")
        storages_data = []
        mes_data = []
        result = []
        try:
            for i in range(len(main_table)):
                storage = main_table[i][0]
                record = main_table[i][1]
                query = f"SELECT * FROM test.{storage}"
                self.cursor.execute(query)
                storages = self.cursor.fetchall()
                storages_data.append({"storage name": storage, "record name": storages[0][1]})
                #print(storages_data)
                
                amount_i = curr[i][1]
                rec_name_i = curr[i][0]

                amount_c = c[i][1]
                rec_name_c =  c[i][0]
                mes_data.append({"number of I(V) chars for given record" : amount_i, "I(V) record": rec_name_i, "number of C(V) chars for given record": amount_c, "C(V) record": rec_name_c})
                
            result.append(mes_data)
            result.append(storages_data)
            #print(mes_data)
            #print(storages_data)

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