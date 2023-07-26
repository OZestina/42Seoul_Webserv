#!/usr/local/bin/python3  

print('''<!doctype html>
<html>
<head>
  <link rel="shortcut icon" href="data:image/x-icon;," type="image/x-icon">
  <title>Welcome to SpiderMen</title>
  <meta charset="utf-8">
</head>
      
<body>
  <h1><a href="http://0.0.0.0:4242/">this is HTML Page</a></h1>
  <ol>
    <li><a href="http://0.0.0.0:4242/index.py">to Python Page</a></li>
    <li><a href="http://0.0.0.0:4242/index.php">to PHP Page</a></li>
  </ol>

  <h1>File send</h1>
  <div class="choices">
	<form action="http://0.0.0.0:4242/upload/" method="POST" enctype="multipart/form-data">
		<input type="file" name="name" id="">
		<button type="upload">submit</button>
	</form>

  <h1>File delete</h1>
  <div class="choices">
	<form action="http://0.0.0.0:4242/upload/" method="DELETE">
		<input type="text" name="name" id="">
		<button type="delete">submit</button>
	</form>

</body>
</html>
''')