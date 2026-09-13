BEGIN TRANSACTION;

INSERT INTO agents (id, name, center_x, center_y, radius) VALUES
    (1, 'Alice', 10, 10, 8),
    (2, 'Bob', 30, 20, 10),
    (3, 'Carol', 15, 30, 12);

INSERT INTO streets (id, name) VALUES
    (1, 'Main Street'),
    (2, 'Oak Street'),
    (3, 'Lake Road'),
    (4, 'Park Avenue'),
    (5, 'Sun Street');

INSERT INTO agent_streets (agent_id, street_id) VALUES
    (1, 1),
    (1, 2),
    (2, 3),
    (2, 4),
    (3, 5),
    (3, 1);

INSERT INTO customers (id, name) VALUES
    (1, 'John Smith'),
    (2, 'Mary Brown'),
    (3, 'Alex Green'),
    (4, 'Diana White'),
    (5, 'George Black');

INSERT INTO addresses (id, customer_id, street_id, number, x, y) VALUES
    (1, 1, 1, '12', 11, 10),
    (2, 2, 3, '5', 30, 22),
    (3, 3, 2, '9', 13, 12),
    (4, 4, 4, '17', 28, 20),
    (5, 5, 5, '2', 16, 31);

INSERT INTO parcels
    (id, tracking_number, address_id, assigned_agent_id, status, delivered_at)
VALUES
    (1, 'PF-1001', 1, 1, 'Assigned', NULL),
    (2, 'PF-1002', 2, 2, 'Assigned', NULL),
    (3, 'PF-1003', 3, 1, 'Delivered', CURRENT_TIMESTAMP),
    (4, 'PF-1004', 4, 2, 'Delivered', CURRENT_TIMESTAMP),
    (5, 'PF-1005', 5, 3, 'Delivered', CURRENT_TIMESTAMP);

INSERT INTO parcel_events (parcel_id, status) VALUES
    (1, 'Created'),
    (1, 'Assigned'),
    (2, 'Created'),
    (2, 'Assigned'),
    (3, 'Created'),
    (3, 'Assigned'),
    (3, 'Delivered'),
    (4, 'Created'),
    (4, 'Assigned'),
    (4, 'Delivered'),
    (5, 'Created'),
    (5, 'Assigned'),
    (5, 'Delivered');

COMMIT;
