import { Router } from "express";
import { z } from "zod";
import { v4 as uuid } from "uuid";
import { JsonStore } from "../services/storage.service.js";
import { PanelGatewayService } from "../services/panel-gateway.service.js";
import type { Teacher } from "../types/domain.js";

export function teachersRouter(store: JsonStore<Teacher[]>, panel: PanelGatewayService) {
  const router = Router();
  router.get("/", async (_req, res) => res.json(await store.get()));
  router.post("/", async (req, res) => {
    const body = z.object({ name: z.string().min(1), subject: z.string().min(1), avatarId: z.string().optional(), profileId: z.string().optional() }).parse(req.body);
    const items = await store.get();
    const item: Teacher = { id: uuid(), name: body.name, subject: body.subject, avatarId: body.avatarId, profileId: body.profileId, createdAt: new Date().toISOString() };
    items.push(item); await store.set(items); panel.sync(); res.status(201).json(item);
  });
  router.patch("/:id/avatar", async (req, res) => {
    const body = z.object({ avatarId: z.string() }).parse(req.body);
    const items = await store.get();
    const item = items.find(x => x.id === req.params.id);
    if (!item) return res.status(404).json({ error: "Teacher not found" });
    item.avatarId = body.avatarId; await store.set(items); panel.sync(); res.json(item);
  });
  router.delete("/:id", async (req, res) => {
    const items = (await store.get()).filter(x => x.id !== req.params.id);
    await store.set(items); panel.sync(); res.json({ ok: true });
  });
  return router;
}
