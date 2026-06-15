import OpenAI from "openai";
import type { Settings, Teacher, Material } from "../types/domain.js";

export class AIService {
  private client: OpenAI | null;
  constructor() {
    this.client = process.env.OPENAI_API_KEY ? new OpenAI({ apiKey: process.env.OPENAI_API_KEY }) : null;
  }

  async answer(question: string, teacher: Teacher, settings: Settings, materials: Material[]): Promise<string> {
    if (process.env.MOCK_AI === "true" || !this.client) {
      return `Larnia odpoveď pre učiteľa ${teacher.name}: ${question}`;
    }

    const materialContext = materials
      .slice(0, 6)
      .map((m, i) => `Materiál ${i + 1} (${m.title}):\n${m.content.slice(0, 2500)}`)
      .join("\n\n");

    const lengthRule = settings.ai.responseLength === "short" ? "Odpovedaj krátko, najviac 5 viet." :
      settings.ai.responseLength === "medium" ? "Odpovedaj primerane podrobne." : "Odpovedaj podrobne, ale stále zrozumiteľne.";

    const prompt = `Si ${settings.ai.assistantName}, AI učiteľ v projekte Larnia.\n` +
      `Učiteľ/predmet: ${teacher.name} (${teacher.subject}).\n` +
      `${lengthRule}\n` +
      `${settings.ai.customInstructions || "Vysvetľuj trpezlivo a školsky."}\n` +
      (settings.ai.useMaterials && materialContext ? `Použi tieto materiály ako kontext:\n${materialContext}\n` : "") +
      `Otázka študenta: ${question}`;

    const response = await this.client.responses.create({
      model: settings.ai.model || process.env.OPENAI_MODEL || "gpt-5.5",
      input: prompt,
      temperature: settings.ai.temperature
    });
    return response.output_text;
  }
}
