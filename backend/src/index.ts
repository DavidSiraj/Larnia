import "dotenv/config";
import express from "express";
import cors from "cors";
import http from "http";
import path from "path";
import { JsonStore } from "./services/storage.service.js";
import { PanelGatewayService } from "./services/panel-gateway.service.js";
import { AIService } from "./services/ai.service.js";
import { profilesRouter } from "./routes/profiles.js";
import { teachersRouter } from "./routes/teachers.js";
import { materialsRouter } from "./routes/materials.js";
import { settingsRouter } from "./routes/settings.js";
import { chatRouter } from "./routes/chat.js";
import { avatarsRouter } from "./routes/avatars.js";
import type { Profile, Teacher, Material, Settings } from "./types/domain.js";

const defaultSettings: Settings = {
  avatar: { selectedAvatarId: "", gender: "", voice: "nova", mouthAnimationEnabled: true, showOnTeacherCard: true },
  ai: { assistantName: "Larnia", model: process.env.OPENAI_MODEL || "gpt-5.5", language: "sk", responseLength: "short", temperature: 0.35, useMaterials: true, modes: ["qa"], customInstructions: "Odpovedaj ako trpezlivý školský učiteľ." },
  general: { wifiSsid: "", wifiPassword: "", backendHost: "", backendPort: 3000, bluetoothEnabled: false, wakeOnTouch: true, autoReconnect: true, autoSleepSeconds: 120 }
};

const app = express();
const server = http.createServer(app);
const panel = new PanelGatewayService();
panel.attach(server);

const profiles = new JsonStore<Profile[]>(path.resolve("data/profiles.json"), []);
const teachers = new JsonStore<Teacher[]>(path.resolve("data/teachers.json"), []);
const materials = new JsonStore<Material[]>(path.resolve("data/materials.json"), []);
const settings = new JsonStore<Settings>(path.resolve("data/settings.json"), defaultSettings);
const ai = new AIService();


panel.onMessage(async (message: any) => {
  if (message?.type !== "chat-question") return;

  const teacherId = String(message.teacherId || "");
  const question = String(message.question || "").trim();

  if (!teacherId || !question) {
    panel.showAnswer("Chýba učiteľ alebo otázka.");
    return;
  }

  const teacher = (await teachers.get()).find(t => t.id === teacherId);
  if (!teacher) {
    panel.showAnswer("Backend nenašiel učiteľa/predmet. Skús ho vytvoriť znova.");
    return;
  }

  const cfg = await settings.get();
  const relevant = (await materials.get()).filter(m => !m.teacherId || m.teacherId === teacher.id || m.subject === teacher.subject);

  try {
    panel.setTalking(true);
    const answer = await ai.answer(question, teacher, cfg, relevant);
    panel.showAnswer(answer, teacher.avatarId || cfg.avatar.selectedAvatarId);
  } catch (err: any) {
    panel.showAnswer("AI chyba: " + (err?.message || "neznáma chyba backendu"));
  } finally {
    panel.setTalking(false);
  }
});


app.use(cors());
app.use(express.json({ limit: "2mb" }));
app.get("/health", (_req, res) => res.json({ ok: true, panels: panel.count() }));
app.use("/api/profiles", profilesRouter(profiles));
app.use("/api/teachers", teachersRouter(teachers, panel));
app.use("/api/materials", materialsRouter(materials));
app.use("/api/settings", settingsRouter(settings, panel));
app.use("/api/chat", chatRouter(ai, teachers, materials, settings, panel));
app.use("/api/avatars", avatarsRouter());
app.get("/api/panel/status", (_req, res) => res.json({ connectedPanels: panel.count() }));
app.post("/api/panel/sync", (_req, res) => { panel.sync(); res.json({ ok: true }); });
app.use((err: any, _req: express.Request, res: express.Response, _next: express.NextFunction) => {
  const message = err?.message || "Server error";
  res.status(err?.status || 400).json({ error: message });
});

const port = Number(process.env.PORT || 3000);
server.listen(port, () => console.log(`Larnia backend running on http://localhost:${port}`));
