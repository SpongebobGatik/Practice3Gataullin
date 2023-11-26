import string
import random
import socket
from flask import Flask, request, redirect
from flask_cors import CORS

app = Flask(__name__)
CORS(app)

@app.route('/form', methods=['GET'])
def form():
    return '''
    <!DOCTYPE html>
<html>
<body>

<h2>Сокращения URL</h2>

<form id="myForm">
  <label for="url">Введите URL:</label><br>
  <input type="text" id="url" name="url"><br>
  <input type="submit" value="Сократить URL">
</form> 

<p id="shortUrl"></p>

<script>
document.getElementById('myForm').addEventListener('submit', function(e) {
  e.preventDefault();
  var url = document.getElementById('url').value;
  if(url.trim() === '') {
    document.getElementById('shortUrl').innerText = 'Пожалуйста, введите URL';
    return;
  }
  if(new Blob([url]).size > 50000) {
    document.getElementById('shortUrl').innerText = 'URL не должен превышать 50000 байт';
    return;
  }
  fetch('/', {
    method: 'POST',
    headers: {
      'Content-Type': 'text/plain'
    },
    body: url
  }).then(function(response) {
    if (!response.ok) {
      throw new Error('Ошибка сети при попытке сократить URL');
    }
    return response.text();
  }).then(function(text) {
    if (text === 'Error.') {
      document.getElementById('shortUrl').innerText = 'Произошла ошибка при обработке вашего запроса';
    } else {
      document.getElementById('shortUrl').innerText = 'Ваш сокращенный URL: ' + text;
    }
  }).catch(function(error) {
    document.getElementById('shortUrl').innerText = 'Произошла ошибка: ' + error.message;
  });
});
</script>

</body>
</html>
    '''

def generate_short_url():
    return ''.join(random.choices(string.ascii_letters + string.digits, k=8))

@app.route('/', methods=['POST'])
def shorten_url():
    url = request.data.decode()
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            try:
                s.connect((SERVER_IP, SERVER_PORT))
            except Exception as e:
                return f'Не удалось подключиться к серверу: {str(e)}', 500
            s.sendall(f'--file urls.data --query HGET short_urls {url}'.encode())
            existing_url = s.recv(1024).decode()
            existing_url = existing_url.replace('\n', '').replace('\r', '')
            if existing_url == "Error.":
                return 'Error.', 500
            if existing_url != "-> False":
                return f'{request.url_root}SU/{existing_url}'
    while True:
        short_url = generate_short_url()
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            try:
                s.connect((SERVER_IP, SERVER_PORT))
            except Exception as e:
                return f'Не удалось подключиться к серверу: {str(e)}', 500
            s.sendall(f'--file urls.data --query HGET urls {short_url}'.encode())
            existing_url = s.recv(1024).decode()
            existing_url = existing_url.replace('\n', '').replace('\r', '')
            if existing_url == "Error.":
                return 'Error.', 500
            if existing_url == "-> False":
                with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                    try:
                        s.connect((SERVER_IP, SERVER_PORT))
                    except Exception as e:
                        return f'Не удалось подключиться к серверу: {str(e)}', 500
                    s.sendall(f'--file urls.data --query HSET urls {url} {short_url}'.encode())
                    if s.recv(1024).decode().replace('\n', '').replace('\r', '') == "Error.":
                        return 'Error.', 500
                with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                    try:
                        s.connect((SERVER_IP, SERVER_PORT))
                    except Exception as e:
                        return f'Не удалось подключиться к серверу: {str(e)}', 500
                    s.sendall(f'--file urls.data --query HSET short_urls {short_url} {url}'.encode())
                    if s.recv(1024).decode().replace('\n', '').replace('\r', '') == "Error.":
                        return 'Error.', 500
                return f'{request.url_root}SU/{short_url}'

@app.route('/SU/<short_url>', methods=['GET'])
def redirect_url(short_url):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        try:
            s.connect((SERVER_IP, SERVER_PORT))
        except Exception as e:
            return f'Не удалось подключиться к серверу: {str(e)}', 500
        s.sendall(f'--file urls.data --query HGET urls {short_url}'.encode())
        url = s.recv(1024).decode()
        url = url.replace('\n', '').replace('\r', '')
        if url == "Error.":
            return 'Error.', 500
    return redirect(url, code=302)

if __name__ == '__main__':
    SERVER_IP, SERVER_PORT = input("Введите IP-адрес и порт сервера (формат: myip:myport): ").split(':')
    SERVER_PORT = int(SERVER_PORT)
    app.run(debug=True)
