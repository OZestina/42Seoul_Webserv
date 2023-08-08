#!/usr/bin/php
<?php
    
$filePath = getenv('DOCUMENT_ROOT');
$command = "/usr/bin/php "; // 실행하고자 하는 명령어
$command .= $filePath;
$result = exec($command, $output, $returnVar);

if ($returnVar === 0) {
    // 명령어 실행 성공
    echo "Content-Type: text/html\n\n";
    echo implode($output);
} else {
    // 명령어 실행 실패
    echo "Command execution failed!";
}