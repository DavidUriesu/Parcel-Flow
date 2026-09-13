PRAGMA foreign_keys = ON;

BEGIN TRANSACTION;

CREATE TABLE IF NOT EXISTS customers (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL CHECK (length(trim(name)) > 0)
);

CREATE TABLE IF NOT EXISTS streets (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL UNIQUE CHECK (length(trim(name)) > 0)
);

CREATE TABLE IF NOT EXISTS agents (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL UNIQUE CHECK (length(trim(name)) > 0),
    center_x INTEGER NOT NULL,
    center_y INTEGER NOT NULL,
    radius INTEGER NOT NULL CHECK (radius >= 0)
);

CREATE TABLE IF NOT EXISTS addresses (
    id INTEGER PRIMARY KEY,
    customer_id INTEGER NOT NULL,
    street_id INTEGER NOT NULL,
    number TEXT NOT NULL CHECK (length(trim(number)) > 0),
    x INTEGER NOT NULL CHECK (x >= 0),
    y INTEGER NOT NULL CHECK (y >= 0),

    FOREIGN KEY (customer_id) REFERENCES customers(id) ON DELETE RESTRICT,
    FOREIGN KEY (street_id) REFERENCES streets(id) ON DELETE RESTRICT
);

CREATE TABLE IF NOT EXISTS agent_streets (
    agent_id INTEGER NOT NULL,
    street_id INTEGER NOT NULL,

    PRIMARY KEY (agent_id, street_id),
    FOREIGN KEY (agent_id) REFERENCES agents(id) ON DELETE CASCADE,
    FOREIGN KEY (street_id) REFERENCES streets(id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS parcels (
    id INTEGER PRIMARY KEY,
    tracking_number TEXT NOT NULL UNIQUE CHECK (length(trim(tracking_number)) > 0),
    address_id INTEGER NOT NULL,
    assigned_agent_id INTEGER,
    status TEXT NOT NULL DEFAULT 'Created'
        CHECK (status IN (
            'Created',
            'Assigned',
            'Delivered'
        )),
    created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
    delivered_at TEXT,

    FOREIGN KEY (address_id) REFERENCES addresses(id) ON DELETE RESTRICT,
    FOREIGN KEY (assigned_agent_id) REFERENCES agents(id) ON DELETE SET NULL
);

CREATE TABLE IF NOT EXISTS parcel_events (
    id INTEGER PRIMARY KEY,
    parcel_id INTEGER NOT NULL,
    status TEXT NOT NULL
        CHECK (status IN (
            'Created',
            'Assigned',
            'Delivered'
        )),
    occurred_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,

    FOREIGN KEY (parcel_id) REFERENCES parcels(id) ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_addresses_customer_id
    ON addresses(customer_id);

CREATE INDEX IF NOT EXISTS idx_addresses_street_id
    ON addresses(street_id);

CREATE INDEX IF NOT EXISTS idx_agent_streets_street_id
    ON agent_streets(street_id);

CREATE INDEX IF NOT EXISTS idx_parcels_address_id
    ON parcels(address_id);

CREATE INDEX IF NOT EXISTS idx_parcels_assigned_agent_id
    ON parcels(assigned_agent_id);

CREATE INDEX IF NOT EXISTS idx_parcels_status
    ON parcels(status);

CREATE INDEX IF NOT EXISTS idx_parcel_events_parcel_id
    ON parcel_events(parcel_id);

PRAGMA user_version = 2;

COMMIT;
