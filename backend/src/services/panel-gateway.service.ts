import { WebSocketServer, WebSocket } from "ws";
import type { Server } from "http";

type PanelMessageHandler = (message: any) => void | Promise<void>;

export class PanelGatewayService {
  private clients = new Set<WebSocket>();
  private handlers: PanelMessageHandler[] = [];

  attach(server: Server) {
    const wss = new WebSocketServer({ server, path: "/crowpanel" });
    wss.on("connection", (socket) => {
      this.clients.add(socket);

      socket.on("message", async (raw) => {
        try {
          const message = JSON.parse(raw.toString());
          for (const handler of this.handlers) await handler(message);
        } catch (err) {
          console.error("Panel message error:", err);
          this.send({ type: "answer", text: "Backend prijal správu, ale nevedel ju spracovať." });
        }
      });

      socket.on("close", () => this.clients.delete(socket));
    });
  }

  onMessage(handler: PanelMessageHandler) {
    this.handlers.push(handler);
  }

  count() { return this.clients.size; }

  send(event: unknown) {
    const text = JSON.stringify(event);
    for (const client of this.clients) if (client.readyState === WebSocket.OPEN) client.send(text);
  }

  wake() { this.send({ type: "show", page: "teachers" }); }
  showAnswer(text: string, avatarId?: string) { this.send({ type: "answer", text, avatarId }); }
  setTalking(state: boolean) { this.send({ type: "talk", state }); }
  sync() { this.send({ type: "sync" }); }
}
