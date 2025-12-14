# AnimalCrud.py

from pymongo import MongoClient
from pymongo.errors import PyMongoError

class AnimalCrud:
   
#Connection variables to access database.
    def __init__(
        self,
        user: str = "aacuser",
        password: str = "Securepasslol",
        host: str = "localhost",
        port: int = 33440,
        db_name: str = "aac",
        collection_name: str = "animals"
    ):
      #Initialization
        try:
            uri = f"mongodb://{user}:{password}@{host}:{port}"
            self.client = MongoClient(uri)
            self.database = self.client[db_name]
            self.collection = self.database[collection_name]
        except PyMongoError as e:
            raise Exception(f"Error: {e}")
	#CREATE
    def create(self, data: dict) -> bool:
        
        if data is None or not isinstance(data, dict) or not data:
            raise ValueError("create(): 'data' must be a non-empty dict")

        try:
            result = self.collection.insert_one(data)
            return bool(result.inserted_id)
        except PyMongoError:
            return False
	#READ
    def read(self, query: dict) -> list: # Attempt to read data.
       
        if query is None or not isinstance(query, dict) or not query:
            raise ValueError("read(): 'query' must be a non-empty dict")

        try:
            cursor = self.collection.find(query) #initialize query
            return [document for document in cursor]
        except PyMongoError:
            return []
            
	  # UPDATE
    def update(self, query: dict, new_values: dict) -> int:
        if not isinstance(query, dict) or not isinstance(new_values, dict):
            raise ValueError("update(): 'query' and 'new_values' must be dictionaries")
        try:
            result = self.collection.update_many(query, {"$set": new_values})
            return result.modified_count
        except PyMongoError as e:
            print(f"Update Error: {e}")
            return 0

    	# DELETE
    def delete(self, query: dict) -> int:
        if not isinstance(query, dict) or not query:
            raise ValueError("delete(): 'query' must be a non-empty dictionary")
        try:
            result = self.collection.delete_many(query)
            return result.deleted_count
        except PyMongoError as e:
            print(f"Delete Error: {e}")
            return 0
