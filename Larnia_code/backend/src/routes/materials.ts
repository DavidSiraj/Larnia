import { Router } from "express";
import multer from "multer";
import { z } from "zod";
import { v4 as uuid } from "uuid";
import pdf from "pdf-parse";
import mammoth from "mammoth";
import { JsonStore } from "../services/storage.service.js";
import type { Material } from "../types/domain.js";

const allowed = new Set(["pdf", "docx", "txt"]);
const upload = multer({ storage: multer.memoryStorage(), limits: { fileSize: 10 * 1024 * 1024 } });

function ext(name: string) { return name.split(".").pop()?.toLowerCase() ?? ""; }

export function materialsRouter(store: JsonStore<Material[]>) {
  const router = Router();
  router.get("/", async (req, res) => {
    const teacherId = req.query.teacherId?.toString();
    const items = await store.get();
    res.json(teacherId ? items.filter(x => !x.teacherId || x.teacherId === teacherId) : items);
  });

  router.post("/url", async (req, res) => {
    const body = z.object({ title: z.string().min(1), url: z.string().url(), teacherId: z.string().optional(), subject: z.string().optional() }).parse(req.body);
    const items = await store.get();
    const item: Material = { id: uuid(), teacherId: body.teacherId, subject: body.subject, title: body.title, type: "url", source: body.url, content: body.url, createdAt: new Date().toISOString() };
    items.push(item); await store.set(items); res.status(201).json(item);
  });

  router.post("/upload", upload.single("file"), async (req, res) => {
    if (!req.file) return res.status(400).json({ error: "Vybrať súbor zo zariadenia (PDF, DOCX, TXT)" });
    const extension = ext(req.file.originalname);
    if (!allowed.has(extension)) return res.status(400).json({ error: "Tento formát nie je podporovaný. Nahrajte súbor vo formáte PDF, DOCX alebo TXT." });
    let content = "";
    if (extension === "txt") content = req.file.buffer.toString("utf8");
    if (extension === "pdf") content = (await pdf(req.file.buffer)).text;
    if (extension === "docx") content = (await mammoth.extractRawText({ buffer: req.file.buffer })).value;
    const items = await store.get();
    const item: Material = { id: uuid(), teacherId: req.body.teacherId, subject: req.body.subject, title: req.file.originalname, type: extension as any, source: req.file.originalname, content, createdAt: new Date().toISOString() };
    items.push(item); await store.set(items); res.status(201).json(item);
  });

  router.delete("/:id", async (req, res) => {
    const items = (await store.get()).filter(x => x.id !== req.params.id);
    await store.set(items); res.json({ ok: true });
  });
  return router;
}
