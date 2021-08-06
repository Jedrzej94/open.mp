#include "databases_component.hpp"

DatabasesComponent databaseComponent;

/// Gets the component name
/// @returns Component name
StringView DatabasesComponent::componentName() {
	return "Databases";
}

/// Gets the component type
/// @returns Component type
ComponentType DatabasesComponent::componentType() {
	return ComponentType::Pool;
}

/// Called for every component after components have been loaded
/// Should be used for storing the core interface, registering player/core event handlers
/// Should NOT be used for interacting with other components as they might not have been initialised yet
void DatabasesComponent::onLoad(ICore* c) {}

/// Opens a new database connection
/// @param path Path to the database
/// @returns Database if successful, otherwise "nullptr"
IDatabaseConnection* DatabasesComponent::open(StringView path) {
	DatabaseConnection* ret(nullptr);
	int database_connection_index(databaseConnections.findFreeIndex());
	// TODO: Properly handle invalid indices
	if (database_connection_index >= 0) {
		sqlite3* database_connection_handle(nullptr);
		// TODO: Properly handle errors
		if (sqlite3_open_v2(path.data(), &database_connection_handle, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr) == SQLITE_OK) {
			database_connection_index = databaseConnections.claim(database_connection_index);
			if (database_connection_index < 0) {
				sqlite3_close_v2(database_connection_handle);
				ret = nullptr;
			}
			else {
				ret = &databaseConnections.get(database_connection_index);
				ret->setDatabaseConnectionHandle(database_connection_handle);
			}
		}
	}
	return ret;
}

/// Closes the specified database connection
/// @param databaseConnection Database connection
/// @returns "true" if database connection has been successfully closed, otherwise "false"
bool DatabasesComponent::close(IDatabaseConnection& connection) {
	int database_connection_index(static_cast<DatabaseConnection&>(connection).poolID);
	bool ret(databaseConnections.valid(database_connection_index));
	if (ret) {
		databaseConnections.get(database_connection_index).close();
		databaseConnections.remove(database_connection_index);
	}
	return ret;
}

/// Gets the number of open database connections
/// @returns Number of open database connections
std::size_t DatabasesComponent::getOpenConnectionCount() const {
	std::size_t res = 0;
	for (IDatabaseConnection* c : databaseConnections.entries()) {
		DatabaseConnection* connection = static_cast<DatabaseConnection*>(c);
		if (connection->databaseConnectionHandle) {
			++res;
		}
	}
	return res;
}

/// Gets the number of open database result sets
/// @returns Number of open database result sets
std::size_t DatabasesComponent::getOpenDatabaseResultSetCount() const {
	std::size_t res = 0;
	for (IDatabaseConnection* connection : databaseConnections.entries()) {
		res += static_cast<DatabaseConnection*>(connection)->resultSets.entries().size();
	}
	return res;
}

/// Check if an index is claimed
/// @param index Index
/// @returns "true" if entry is valid, otherwise "false"
bool DatabasesComponent::valid(int index) const {
	if (index == 0) {
		return false;
	}
	return databaseConnections.valid(index);
}

/// Get the object at an index
IDatabaseConnection& DatabasesComponent::get(int index) {
	return databaseConnections.get(index);
}

/// Get a set of all the available objects
const FlatPtrHashSet<IDatabaseConnection>& DatabasesComponent::entries() {
	return databaseConnections.entries();
}

/// Finds the first free index
/// @returns Free index or -1 if no index is available to use
int DatabasesComponent::findFreeIndex() {
	return databaseConnections.findFreeIndex();
}

/// Claims the first free index
/// @returns Claimed index or -1 if no index is available to use
int DatabasesComponent::claim() {
	return databaseConnections.claim();
}

/// Attempts to claim the index at hint and if unavailable, claim the first available index
/// @param hint Hint index
/// @returns Claimed index or -1 if no index is available to use
int DatabasesComponent::claim(int hint) {
	return databaseConnections.claim(hint);
}

/// Releases the object at the specified index
/// @param index Index
void DatabasesComponent::release(int index) {
	databaseConnections.release(index, true);
}

/// Locks an entry at index to postpone release until unlocked
/// @param index Index
void DatabasesComponent::lock(int index) {
	databaseConnections.lock(index);
}

/// Unlocks an entry at index and release it if needed
/// @param index Index
void DatabasesComponent::unlock(int index) {
	databaseConnections.unlock(index);
}

COMPONENT_ENTRY_POINT() {
	return &databaseComponent;
}
