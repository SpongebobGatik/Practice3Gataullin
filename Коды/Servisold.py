import string
import random
import socket
from flask import Flask, request, redirect
from flask_cors import CORS

app = Flask(__name__)  # Создание экземпляра приложения Flask
CORS(app)  # Разрешение CORS для всего приложения

# Декоратор для обработки GET-запроса по пути '/form'
@app.route('/form', methods=['GET'])
def form():
    # Возвращает HTML-форму для ввода URL
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

# Функция для генерации короткого URL
def generate_short_url():
    return ''.join(random.choices(string.ascii_letters + string.digits, k=8))

# Декоратор для обработки POST-запроса по корневому пути '/'
@app.route('/', methods=['POST'])
def shorten_url():
    # Получение данных из запроса и декодирование
    url = request.data.decode()
    # Создание сокета для подключения к серверу
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        try:
            # Попытка подключения к серверу
            s.connect((SERVER_IP, SERVER_PORT))
        except Exception as e:
            # В случае ошибки возвращается сообщение об ошибке
            return f'Не удалось подключиться к серверу: {str(e)}', 500
        # Отправка запроса на получение существующего короткого URL
        s.sendall(f'--file urls.data --query HGET short_urls {url}'.encode())
        # Получение ответа от сервера
        existing_url = s.recv(1024).decode()
        existing_url = existing_url.replace('\n', '').replace('\r', '')
        # Обработка ошибки или возврат существующего короткого URL
        if existing_url == "Error.":
            s.close()
            return 'Error.', 501
        if existing_url != "-> False":
            s.close()
            return f'{request.url_root}SU/{existing_url}'
        # Генерация нового короткого URL, если существующего нет
        while True:
            short_url = generate_short_url()
            s.sendall(f'--file urls.data --query HGET urls {short_url}'.encode())
            existing_url = s.recv(1024).decode()
            existing_url = existing_url.replace('\n', '').replace('\r', '')
            if existing_url == "Error.":
                s.close()
                return 'Error.', 502
            if existing_url == "-> False":
                s.sendall(f'--file urls.data --query HSET urls {url} {short_url}'.encode())
                if s.recv(1024).decode().replace('\n', '').replace('\r', '') == "Error.":
                    s.close()
                    return 'Error.', 503
                s.sendall(f'--file urls.data --query HSET short_urls {short_url} {url}'.encode())
                if s.recv(1024).decode().replace('\n', '').replace('\r', '') == "Error.":
                    s.close()
                    return 'Error.', 504
                s.close()
                return f'{request.url_root}SU/{short_url}'

# Декоратор для обработки GET-запроса по пути '/SU/<short_url>'
@app.route('/SU/<short_url>', methods=['GET'])
def redirect_url(short_url):
    # Создание сокета для подключения к серверу
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        try:
            # Попытка подключения к серверу
            s.connect((SERVER_IP, SERVER_PORT))
        except Exception as e:
            # В случае ошибки возвращается сообщение об ошибке
            return f'Не удалось подключиться к серверу: {str(e)}', 505
        # Отправка запроса на получение оригинального URL
        s.sendall(f'--file urls.data --query HGET urls {short_url}'.encode())
        # Получение ответа от сервера
        url = s.recv(1024).decode()
        url = url.replace('\n', '').replace('\r', '')
        # Обработка ошибки или перенаправление на оригинальный URL
        if url == "Error.":
            s.close()
            return 'Error.', 506
        s.close()
    return redirect(url, code=302)

if __name__ == '__main__':
    SERVER_IP, SERVER_PORT = input("Введите IP-адрес и порт сервера (формат: myip:myport): ").split(':')
    SERVER_PORT = int(SERVER_PORT)
    app.run(debug=True)
