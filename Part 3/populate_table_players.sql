DELIMITER //
-- Заполняет таблицу players случайными игроками (name) из пулла имен
-- и случайными датами входа (login)
CREATE PROCEDURE populate_table_players(IN req_num_players INT)
BEGIN
  DECLARE AVAILABLE_PLAYER_NAMES_COUNT INT DEFAULT 15; 
  DECLARE MAX_LAST_LOGIN_OFFSET_DAYS INT DEFAULT 10;  -- в днях
  DECLARE MAX_DEVICE INT DEFAULT 30;

  DECLARE cur_num_players INT DEFAULT 0;

  DECLARE new_player_name VARCHAR(64);
  DECLARE new_player_last_login_offset_days INT;
  DECLARE new_player_last_login_hour INT;
  DECLARE new_player_last_login_minute INT;
  DECLARE new_player_last_login_second INT;
  DECLARE new_player_last_login DATETIME;
  DECLARE new_player_device INT;

  WHILE cur_num_players < req_num_players DO
    SET new_player_name = ELT(FLOOR(1 + RAND() * AVAILABLE_PLAYER_NAMES_COUNT), 
                        'Alex', 'Bob', 'Charlie', 'Henry', 'Michael', 
                        'Olga', 'Oleg', 'Peter', 'Victor', 'Lena',
                        'Karl', 'Philip', 'Dmitry', 'Yaroslav', 'Sergey');

    SET new_player_last_login_offset_days = FLOOR(RAND() * MAX_LAST_LOGIN_OFFSET_DAYS);
    SET new_player_last_login_hour = FLOOR(RAND() * 24);
    SET new_player_last_login_minute = FLOOR(RAND() * 60);
    SET new_player_last_login_second = FLOOR(RAND() * 60);

    SET new_player_last_login = NOW() - INTERVAL new_player_last_login_offset_days DAY
                        - INTERVAL HOUR(NOW()) HOUR
                        - INTERVAL MINUTE(NOW()) MINUTE
                        - INTERVAL SECOND(NOW()) SECOND
                        + INTERVAL new_player_last_login_hour HOUR
                        + INTERVAL new_player_last_login_minute MINUTE
                        + INTERVAL new_player_last_login_second SECOND;

    SET new_player_device = FLOOR(RAND() * MAX_DEVICE);

    INSERT INTO players (`name`, `login_time`, `device`)
    VALUES (new_player_name, new_player_last_login, new_player_device);

    SET cur_num_players = cur_num_players + 1;
  END WHILE;
END //

DELIMITER ;