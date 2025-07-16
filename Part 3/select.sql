-- Получить 5 самых активных устройств – 
-- то есть пять значений поля device, 
-- с которых произошло наибольшее количество логинов пользователей 
-- (упорядочить по количеству логинов)
SELECT `device`, COUNT(*) AS `login_count`
FROM players
GROUP BY `device`
ORDER BY 2 DESC
LIMIT 5;

-- Получить **среднее число логинов в день** за последние 7 дней 
-- (в расчете от текущей даты)
SELECT ROUND(COUNT(*) / COUNT(DISTINCT DATE(`login_time`)), 2) AS `avg_logins_per_day`
FROM players
WHERE `login_time` >= CURDATE() - INTERVAL 6 DAY;