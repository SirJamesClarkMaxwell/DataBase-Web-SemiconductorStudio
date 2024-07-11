import psycopg2
import json


conn = psycopg2.connect(
    dbname='semiconductor',
    user='postgres',
    password='jebacUW',
    host='localhost',
    port='5173',
    options = "-c search_path=test"  
)

cursor = conn.cursor()

def get_tables(cursor):
    cursor.execute("""
        SELECT table_name
        FROM information_schema.tables
        WHERE table_schema = 'public'
    """)
    return [row[0] for row in cursor.fetchall()]

def get_columns(cursor, table_name):
    cursor.execute(f"""
        SELECT column_name, data_type, is_nullable, column_default
        FROM information_schema.columns
        WHERE table_name = '{table_name}'
    """)
    columns = []
    for row in cursor.fetchall():
        columns.append({
            'name': row[0],
            'type': row[1],
            'nullable': row[2],
            'default': row[3]
        })
    return columns

def get_relationships(cursor):
    cursor.execute("""
        SELECT
            tc.table_name AS table_name,
            kcu.column_name AS column_name,
            ccu.table_name AS foreign_table_name,
            ccu.column_name AS foreign_column_name
        FROM 
            information_schema.table_constraints AS tc 
            JOIN information_schema.key_column_usage AS kcu
              ON tc.constraint_name = kcu.constraint_name
              AND tc.table_schema = kcu.table_schema
            JOIN information_schema.constraint_column_usage AS ccu
              ON ccu.constraint_name = tc.constraint_name
              AND ccu.table_schema = tc.table_schema
        WHERE tc.constraint_type = 'FOREIGN KEY'
    """)
    relationships = []
    for row in cursor.fetchall():
        relationships.append({
            'table': row[0],
            'column': row[1],
            'foreign_table': row[2],
            'foreign_column': row[3]
        })
    return relationships

database_structure = {}
tables = get_tables(cursor)
for table in tables:
    database_structure[table] = {
        'columns': get_columns(cursor, table)
    }


database_structure['relationships'] = get_relationships(cursor)

cursor.close()
conn.close()

with open('Web-DataBase-SemiconductorStudio\python-hub\database_structure.json', 'w') as json_file:
    json.dump(database_structure, json_file, indent=4)

print("Struktura bazy danych została zapisana do pliku 'database_structure.json'.")
