import { Router } from "express";
import { z } from "zod";
import { v4 as uuid } from "uuid";
import { JsonStore } from "../services/storage.service.js";
import type { Profile } from "../types/domain.js";

export function profilesRouter(store: JsonStore<Profile[]>) {
  const router = Router();
  router.get("/", async (_req, res) => res.json(await store.get()));
  router.post("/", async (req, res) => {
    const body = z.object({ name: z.string().min(1), age: z.coerce.number().int().min(0).max(120), avatarId: z.string().optional() }).parse(req.body);
    const items = await store.get();
    const item: Profile = { id: uuid(), name: body.name, age: body.age, avatarId: body.avatarId, createdAt: new Date().toISOString() };
    items.push(item); await store.set(items); res.status(201).json(item);
  });
  router.delete("/:id", async (req, res) => {
    const items = (await store.get()).filter(x => x.id !== req.params.id);
    await store.set(items); res.json({ ok: true });
  });
  return router;
}
