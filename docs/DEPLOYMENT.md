# FEmmg-FHE Deployment Guide

## Production Deployment

### Option 1: Nginx Reverse Proxy (Recommended)

```bash
# Install nginx and certbot
sudo apt install nginx certbot python3-certbot-nginx

# Get SSL certificate
sudo certbot --nginx -d yourdomain.com

# Configure nginx
sudo tee /etc/nginx/sites-available/femmg << 'NGINX'
server {
    listen 443 ssl http2;
    server_name yourdomain.com;

    ssl_certificate /etc/letsencrypt/live/yourdomain.com/fullchain.pem;
    ssl_certificate_key /etc/letsencrypt/live/yourdomain.com/privkey.pem;

    location / {
        proxy_pass http://localhost:8092;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
    }
}

server {
    listen 80;
    server_name yourdomain.com;
    return 301 https://$server_name$request_uri;
}
NGINX

# Enable and restart
sudo ln -s /etc/nginx/sites-available/femmg /etc/nginx/sites-enabled/
sudo nginx -t && sudo systemctl reload nginx
```

### Option 2: Docker with Built-in TLS

```bash
# Generate self-signed certificate (development)
openssl req -x509 -newkey rsa:4096 -keyout server.key -out server.crt -days 365 -nodes

# Run with TLS
docker run -d \
  -p 443:8092 \
  -v $(pwd)/server.crt:/app/server.crt \
  -v $(pwd)/server.key:/app/server.key \
  ghcr.io/primordialomegazero/femmgfhe:v22.0.0 \
  ./femmg_server --tls --cert server.crt --key server.key
```

### Option 3: Cloudflare Tunnel (Zero Config)

```bash
# Install cloudflared
curl -L https://github.com/cloudflare/cloudflared/releases/latest/download/cloudflared-linux-amd64 -o cloudflared
chmod +x cloudflared

# Create tunnel
./cloudflared tunnel create femmg-fhe
./cloudflared tunnel route dns femmg-fhe yourdomain.com

# Run tunnel
./cloudflared tunnel run --url http://localhost:8092 femmg-fhe
```

## Security Notes

| Layer | Mechanism |
|-------|-----------|
| **Application** | Golden Chaos CTU v4 (IND-CPA) |
| **Computation** | Fully Blind (server never sees plaintext) |
| **Transport** | TLS 1.3 (via Nginx/Docker/Cloudflare) |
| **Storage** | SpiralDB Lite (encrypted at rest) |

> **Note:** FEmmg-FHE ciphertexts are already IND-CPA secure. TLS provides defense-in-depth for metadata protection (client_id, session tokens, etc.).
