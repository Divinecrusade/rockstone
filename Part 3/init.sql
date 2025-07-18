--MySQL 9.3
CREATE DATABASE IF NOT EXISTS rockstone_divinecrusade;
USE rockstone_divinecrusade

DROP TABLE IF EXISTS players;

CREATE TABLE players(
	`id` INT AUTO_INCREMENT PRIMARY KEY, 
	`name` VARCHAR(64) NOT NULL, 
	`login_time` DATETIME NOT NULL, 
	`device` INT NOT NULL
);

CREATE INDEX idx_device ON players(`device`);
CREATE INDEX idx_login_time ON players(`login_time`);

CALL populate_table_players(50000); -- см. файл populate_table_players.sql