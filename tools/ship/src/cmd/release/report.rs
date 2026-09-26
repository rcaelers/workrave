//! Wall-clock timings for a release, printed after execution and cleanup.

use std::fmt::Write;
use std::time::Duration;

use anyhow::Result;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub(super) enum Status {
    Ok,
    Failed,
    Skipped,
    Echoed,
}

impl Status {
    pub fn from_result<T>(result: &Result<T>) -> Self {
        if result.is_ok() {
            Self::Ok
        } else {
            Self::Failed
        }
    }

    fn label(self) -> &'static str {
        match self {
            Self::Ok => "ok",
            Self::Failed => "failed",
            Self::Skipped => "skipped",
            Self::Echoed => "echoed",
        }
    }
}

pub(super) struct Timing {
    pub label: String,
    pub status: Status,
    /// Skipped steps have no execution time.
    pub elapsed: Option<Duration>,
}

impl Timing {
    pub fn skipped(label: String) -> Self {
        Self {
            label,
            status: Status::Skipped,
            elapsed: None,
        }
    }
}

pub(super) struct JobReport {
    pub timing: Timing,
    pub steps: Vec<Timing>,
}

#[derive(Default)]
pub(super) struct Report {
    pub jobs: Vec<JobReport>,
}

impl Report {
    pub fn render(&self, elapsed: Duration, status: Status, dry_run: bool) -> String {
        let mode = if dry_run { " (dry run)" } else { "" };
        let mut output = format!("\nBuild report{mode}\n");
        writeln!(output, "{:<8}  {:>12}  Job / step", "Status", "Time").unwrap();
        for job in &self.jobs {
            write_timing(&mut output, &job.timing, "");
            for step in &job.steps {
                write_timing(&mut output, step, "  ");
            }
        }
        writeln!(
            output,
            "\nTotal: {} ({})",
            format_duration(elapsed),
            status.label()
        )
        .unwrap();
        crate::system::process::redacted(&output)
    }
}

fn write_timing(output: &mut String, timing: &Timing, indent: &str) {
    let elapsed = timing
        .elapsed
        .map(format_duration)
        .unwrap_or_else(|| "-".to_string());
    // Keep names and matrix values on one line, even when they contain newlines.
    let label = timing
        .label
        .split_whitespace()
        .collect::<Vec<_>>()
        .join(" ");
    writeln!(
        output,
        "{:<8}  {:>12}  {indent}{label}",
        timing.status.label(),
        elapsed
    )
    .unwrap();
}

fn format_duration(elapsed: Duration) -> String {
    let seconds = elapsed.as_secs();
    if seconds >= 3600 {
        format!(
            "{}h {:02}m {:02}s",
            seconds / 3600,
            seconds / 60 % 60,
            seconds % 60
        )
    } else if seconds >= 60 {
        format!("{}m {:02}s", seconds / 60, seconds % 60)
    } else {
        format!("{:.3}s", elapsed.as_secs_f64())
    }
}
