-- phpMyAdmin SQL Dump
-- version 4.4.15.5
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: Apr 05, 2016 at 04:45 PM
-- Server version: 5.5.48
-- PHP Version: 5.4.38

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";

--
-- Database: `ydd_50`
--

DELIMITER $$
--
-- Procedures
--
CREATE DEFINER=`root`@`localhost` PROCEDURE `sp_add_log_record`(IN `p_taskid` INT UNSIGNED, IN `p_type` TINYINT UNSIGNED, IN `p_message` VARCHAR(4096), IN `p_data` MEDIUMTEXT, OUT `p_result` TINYINT UNSIGNED)
        NO SQL
	BEGIN
	DECLARE EXIT HANDLER FOR SQLEXCEPTION ROLLBACK;
	DECLARE EXIT HANDLER FOR SQLWARNING ROLLBACK;

	START TRANSACTION;
	SET p_result = 0;

	INSERT INTO `tasks_logs_records`(`taskid`, `message`, `type`)
	VALUES(p_taskid, p_message, p_type);

	IF (p_data IS NOT NULL) THEN
	    SET @rec_id = LAST_INSERT_ID();
		INSERT INTO `tasks_logs_html`(`recordid`, `data`)
	    VALUES(@rec_id, p_data);

		UPDATE `tasks_logs_records`
			SET `htmlid` = LAST_INSERT_ID()
	    WHERE `id` = @rec_id;
	    END IF;

	    SET p_result = 1;
	    COMMIT;
	    END$$

	    CREATE DEFINER=`root`@`localhost` PROCEDURE `sp_fill_test_tasks_phrases`()
        NO SQL
	BEGIN    
	    DELETE FROM tasks_phrases;
	        
	        
	        # Filling test tasks phrases for taskid = 1
	        # -- These are 72 phrases which are already "processed"
	        CALL sp_fill_test_tasks_phrases_set(1001, 1072, 1, 1);
		    # -- And these are remaining up to 310 phrases which 
		    # -- are pending processing
		    CALL sp_fill_test_tasks_phrases_set(1073, 1310, 0, 1);
		        
		        CALL sp_fill_test_tasks_phrases_set(2001, 2030, 1, 2);
			    CALL sp_fill_test_tasks_phrases_set(2031, 2090, 0, 2);
			        
			        
			        CALL sp_fill_test_tasks_phrases_set(3001, 3050, 1, 3);
				    CALL sp_fill_test_tasks_phrases_set(3051, 3201, 0, 3);
				        
				        
				        CALL sp_fill_test_tasks_phrases_set(4001, 4101, 1, 4);
					    CALL sp_fill_test_tasks_phrases_set(4102, 4249, 0, 4);
					        
					        CALL sp_fill_test_tasks_phrases_set(5001, 5030, 0, 5);
						END$$

						CREATE DEFINER=`root`@`localhost` PROCEDURE `sp_fill_test_tasks_phrases_set`(IN `p_first` INT UNSIGNED, IN `p_last` INT UNSIGNED, IN `p_processed` BOOLEAN, IN `p_taskid` INT UNSIGNED)
        NO SQL
	BEGIN
	    DECLARE i INT;
	        DECLARE phr VARCHAR(32);
		    
		    SET i = p_first;
		        WHILE i <= p_last DO
			        SET phr = CONCAT('phrase', i);
				        INSERT INTO tasks_phrases(`taskid`, `phrase`, `finished`) 
		VALUES (p_taskid, phr, p_processed);
			SET i = i + 1;
			    END WHILE;
			    END$$

			    DELIMITER ;

			    -- --------------------------------------------------------

			    --
			    -- Table structure for table `tasks_phrases`
			    --

			    CREATE TABLE IF NOT EXISTS `tasks_phrases` (
				      `taskid` int(10) unsigned NOT NULL,
				        `phrase` varchar(1024) COLLATE utf8_unicode_ci NOT NULL,
					  `id` int(10) unsigned NOT NULL,
					    `finished` tinyint(1) DEFAULT NULL
				    ) ENGINE=InnoDB AUTO_INCREMENT=1618 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

			    --
			    -- Indexes for dumped tables
			    --

			    --
			    -- Indexes for table `tasks_phrases`
			    --
			    ALTER TABLE `tasks_phrases`
			      ADD PRIMARY KEY (`id`),
			        ADD KEY `taskid` (`taskid`),
				  ADD KEY `finished` (`finished`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `tasks_phrases`
--
ALTER TABLE `tasks_phrases`
  MODIFY `id` int(10) unsigned NOT NULL AUTO_INCREMENT,AUTO_INCREMENT=1618;
