import { sections } from "./presentation-data";

const sourceDocument =
  "https://github.com/PlusLabUChile/3D-GPolylla/blob/main/docs/superpowers/specs/2026-07-28-gpolylla-redesign-design.md";

export default function Home() {
  return (
    <>
      <a className="skip-link" href="#content">
        Saltar al contenido
      </a>
      <nav aria-label="Índice de capítulos">
        <ol>
          {sections.map((section) => (
            <li key={section.id}>
              <a href={`#${section.id}`}>{section.label}</a>
            </li>
          ))}
        </ol>
      </nav>
      <main id="content">
        {sections.map((section) => (
          <section
            aria-labelledby={`${section.id}-title`}
            data-presentation-section
            id={section.id}
            key={section.id}
          >
            <p>Capítulo {String(section.index).padStart(2, "0")}</p>
            <h2 id={`${section.id}-title`}>{section.title}</h2>
            <p>{section.thesis}</p>
            <ul>
              {section.bullets.map((bullet) => (
                <li key={bullet}>{bullet}</li>
              ))}
            </ul>
            {section.detail ? (
              <details>
                <summary>Profundizar</summary>
                <p>{section.detail}</p>
              </details>
            ) : null}
          </section>
        ))}
        <footer>
          <a href={sourceDocument}>Leer el documento técnico completo</a>
        </footer>
      </main>
    </>
  );
}
