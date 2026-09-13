BEGIN TRANSACTION;

INSERT INTO agents (name, center_x, center_y, radius) VALUES
    ('Alice', 10, 10, 8),
    ('Bob', 30, 20, 10),
    ('Carol', 15, 30, 12);

INSERT INTO streets (name, city) VALUES
    ('Main Street', 'Cluj-Napoca'),
    ('Oak Street', 'Cluj-Napoca'),
    ('Lake Road', 'Cluj-Napoca'),
    ('Park Avenue', 'Cluj-Napoca'),
    ('Sun Street', 'Cluj-Napoca');

INSERT INTO agent_streets (agent_id, street_id)
SELECT a.id, s.id FROM agents a CROSS JOIN streets s
WHERE (a.name = 'Alice' AND s.name IN ('Main Street', 'Oak Street'))
   OR (a.name = 'Bob' AND s.name IN ('Lake Road', 'Park Avenue'))
   OR (a.name = 'Carol' AND s.name IN ('Sun Street', 'Main Street'));

INSERT INTO customers (name) VALUES
    ('John Smith'),
    ('Mary Brown'),
    ('Alex Green'),
    ('Diana White'),
    ('George Black');

INSERT INTO addresses (customer_id, street_id, number, x, y)
SELECT c.id, s.id, data.number, data.x, data.y
FROM (
    SELECT 'John Smith' customer, 'Main Street' street, '12' number, 11 x, 10 y
    UNION ALL SELECT 'Mary Brown', 'Lake Road', '5', 30, 22
    UNION ALL SELECT 'Alex Green', 'Oak Street', '9', 13, 12
    UNION ALL SELECT 'Diana White', 'Park Avenue', '17', 28, 20
    UNION ALL SELECT 'George Black', 'Sun Street', '2', 16, 31
) data
JOIN customers c ON c.name = data.customer
JOIN streets s ON s.name = data.street AND s.city = 'Cluj-Napoca';

INSERT INTO parcels (tracking_number, address_id, status, delivered_at)
SELECT data.tracking, a.id, data.status,
       CASE WHEN data.status = 'Delivered' THEN CURRENT_TIMESTAMP END
FROM (
    SELECT 'PF-1001' tracking, 'John Smith' customer, 'Main Street' street, '12' number, 'Created' status
    UNION ALL SELECT 'PF-1002', 'Mary Brown', 'Lake Road', '5', 'Created'
    UNION ALL SELECT 'PF-1003', 'Alex Green', 'Oak Street', '9', 'Delivered'
    UNION ALL SELECT 'PF-1004', 'Diana White', 'Park Avenue', '17', 'Delivered'
    UNION ALL SELECT 'PF-1005', 'George Black', 'Sun Street', '2', 'Delivered'
) data
JOIN customers c ON c.name = data.customer
JOIN streets s ON s.name = data.street AND s.city = 'Cluj-Napoca'
JOIN addresses a ON a.customer_id = c.id AND a.street_id = s.id AND a.number = data.number;

INSERT INTO parcel_events (parcel_id, status)
SELECT id, status FROM parcels;

COMMIT;
